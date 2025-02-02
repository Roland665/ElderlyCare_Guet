import cv2 as cv
from face_recognition import recognition
import serial
import time
from threading import Thread

''' 实例化串口对象，使用GPIO串口 '''
srl = serial.Serial("/dev/ttyAMA0", 115200)

''' 人脸识别用 '''
face_detect_model = "home/pi/Desktop/doorlock/haarcascade_frontalface_default.xml" # 人脸检测模型路径
face_model_file = "home/pi/Desktop/doorlock/train.yml" # 识别模型路径
face_rec = recognition.FaceRec(cv.VideoCapture(0), face_detect_model, "home/pi/Desktop/doorlock/source", srl) # 实例化识别器，并传递串口对象作为调试输出口
rec_flag = False # 识别启用标志位，置一则启用

debug = "[debug]: " # 调试用前缀


'''串口数据分析函数'''
def serial_handle(data :bytes, count :int):
  global rec_flag
  if count <= 1 :
    print(debug+"the count is invalid")
    return
  print(debug+"[serial_handle]: data[1]="+(str)(data[1]))
  if data[1] == 0x00:
    rec_flag = False
  elif data[1] == 0x01:
    rec_flag = True
  elif data[1] == 0x02:
    face_rec.add_face(data[2:(data[0]+1)].decode(), 20)
    face_rec.train(face_model_file)
    srl.write(bytes([0x01, 0x02])) # 通知esp32人脸录入完毕

''' 串口接收处理线程 '''
def serial_handle_task():
  print(debug+'into -> serial_handle_task()')
  '''检测串口是否打开'''
  if srl.isOpen():
    print(debug+"Successfully open serial: ")
    print(srl)
    srl.write(b"pi")
    srl.flush()
  else:
    print(debug+"Serial open failed")

  '''等待esp32发起控制信号'''
  while True:
    count = 0
    while(True):
      if count < srl.inWaiting():
        # 收到了新消息
        time.sleep(0.01) # 等待10ms
        count = srl.inWaiting()
        if(count is srl.inWaiting()): # 如果10ms后依旧没有收到新消息，则确认接收完一帧数据
          data = srl.read(count)
          print("recv: " + (str)(data))
          serial_handle(data, count)
          break


''' 人脸识别线程 '''
def face_recognition_task():
  global rec_flag
  print(debug+'into -> face_recognition_task()')
  face_rec.recognizer.read(face_model_file) # 读取模型文件
  while(True):
    if rec_flag:
      face_names = face_rec.get_face_names() # 获取人脸库所有人脸名称
      time_old = time.time() # 记录时间
      print(debug+"start recognition")
      for i in range(16):
        _, img = face_rec.cam.read() # 清空缓冲区
      while(time.time() - time_old < 10) and rec_flag: # 检测10秒或被强制停止或检查成功人脸
        _, img = face_rec.cam.read()
        ret, rect = face_rec.isFace(img)
        if ret :
          # 预测
          label = face_rec.recognizer.predict(cv.cvtColor(img[rect[1]:rect[1]+rect[3], rect[0]:rect[0]+rect[2]], cv.COLOR_BGR2GRAY))
          print(debug+(str)(label)+"\t\t"+face_names[label[0]]) # label[0]为标签号，label[1]为可信值(数值越低，可信度越高)
          if label[1] < 50:
            print(debug+"成功识别人脸："+face_names[label[0]])
            '''通知esp识别成功，解锁'''
            data = bytearray([0x01])
            data.extend(bytes(face_names[label[0]], "utf-8")) # 将字符以utf-8编码后装载入数据帧
            data.insert(0, len(data))
            srl.write(data)
            rec_flag = False
            break
          time.sleep(0.01)
      if(rec_flag):
        rec_flag = False
        srl.write(bytes([0x01, 0x00])) # 通知esp人脸识别失败
        print(debug+"recognition failed")

''' 主线程 '''
def main():
  print(debug+'into -> main()')
  ''' 创建线程 '''
  task2 = Thread(target=face_recognition_task, args=())
  task1 = Thread(target=serial_handle_task, args=())

  ''' 启动线程 '''
  task1.start()
  task2.start()

if __name__ == '__main__':
  main()
