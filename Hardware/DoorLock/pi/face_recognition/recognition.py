'''人脸识别库，基于OpenCV实现人脸检测——识别'''
import os
import shutil
import time
import cv2 as cv
import numpy as np
import serial

src_path = "source" # 照片存放路径顶级，下一级以人名为路径名的文件夹，再下一级为 人名[i].jpg

class FaceRec:
  cam :cv.VideoCapture # 视频流对象
  face_detector :cv.CascadeClassifier # 人脸检测级联分类器
  recognizer = cv.face.LBPHFaceRecognizer.create() # 人脸识别训练器
  src_path :str # 人脸照片存放路径顶级文件夹名，下一级以人名为路径名的文件夹，再下一级为 人名[i].jpg
  srl :serial # 串口对象，可调试打印使用
  '''
  cam                 : 视频流采集对象，一般由cv2.VideoCapture(0)产生
  face_detect_filePath: 人脸检测模型文件路径
  src_path            : 人脸照片存放路径顶级，下一级以人名为路径名的文件夹，再下一级为 人名[i].jpg
  '''
  def __init__(self, cam :cv.VideoCapture, face_detect_filePath :str, src_path :str, srl :serial):
    self.cam = cam
    self.face_detector = cv.CascadeClassifier(face_detect_filePath) # 用路径下的人脸模型构建级联分类器
    self.src_path = src_path
    self.srl = srl


  '''
  根据人脸模型进行人脸识别
  model_file: 人脸识别模型的路径
  '''
  def recognition(self, model_file:str):
    self.recognizer.read(model_file)
    face_names = self.get_face_names()
    time_old = time.time()
    while(time.time() - time_old < 10): # 检测10秒
      _, img = self.cam.read()
      ret, rect = self.isFace(img)
      if ret :
        # 预测
        label = self.recognizer.predict(cv.cvtColor(img[rect[1]:rect[1]+rect[3], rect[0]:rect[0]+rect[2]], cv.COLOR_BGR2GRAY))
        print((str)(label)+"\t\t"+face_names[label[0]]) # label[0]为标签号，label[1]为可信值(数值越低，可信度越高)
        time.sleep(0.01)



  '''
  添加人脸照片至数据库
  '''
  def add_face(self, name:str, num:int):
    if(os.path.exists(self.src_path + "/" + name)): # 如果已经有了对应人脸的文件，则先清空
      shutil.rmtree(self.src_path + "/" + name)
    os.mkdir(self.src_path + "/" + name) # 新建文件夹
    faces = self.get_faces_fromCam(num)
    for i in range(len(faces)):
      cv.imwrite(self.src_path + "/" + name + "/" + name + (str)(i) + ".jpg", faces[i])


  '''
  人脸检测
  image: 待检测图像
  return: 有两个返回值
          如果图像内有人脸，则返回 True, 人脸在图像位置(x, y, w, h)[xy为矩形左上角坐标，wh为矩形的宽高(因为返回是正方形，所以w=h)]
          如果图像内无人脸，则返回 False，None
  '''
  def isFace(self, image):
      gray = cv.cvtColor(image, cv.COLOR_BGR2GRAY) # 取灰度图像
      faces = self.face_detector.detectMultiScale(gray, 1.2, 6) # 获取检测到的所有人脸位置信息
      # 如果未检测到面部，则返回原始图像
      if (len(faces) == 0):
          return False, None
      # 返回图像中第一张脸部部分
      return True, faces[0]


  '''
  从摄像头获取x张人脸图片，并将有人脸信息的图像存入列表返回
  num: 获取照片数
  '''
  def get_faces_fromCam(self, num:int):
    faces = []
    counter = 0
    temp_time = time.time()
    for i in range(16):
      _, img = self.cam.read() # 清空缓冲区
    while(True): # 循环采集摄像头内图片，找到人脸
      self.srl.write(bytes([0x02, 0x03, (int)(counter*100/num)]))
      _, img = self.cam.read()
      ret, rect = self.isFace(img)
      # 注意：人脸检测后返回的图像大小不一致，需要统一大小(在主程序中处理)
      # 我们忽略未检测到的脸部
      if ret and time.time()-temp_time > 0.05: # 50ms 抓取一次照片
        temp_time = time.time()
        counter+=1
        # 将脸添加到脸部列表并添加相应的标签
        faces.append(img)
        print("Get face num="+(str)(counter))
        if(counter == num):
          break
      time.sleep(0.05)
    return faces

  def get_face_names(self)->list[str]:
    face_names = [] # 存放所有人的名称
    for dir in os.listdir(self.src_path):
      if os.path.isdir(os.path.join(self.src_path, dir)):
        face_names.append(dir)
    return face_names
  '''
  训练人脸识别模型
  fileName: 模型输出文件名
  '''
  def train(self, fileName):
    faces = [] # 存放所有人脸，作为训练数据
    face_names = self.get_face_names() # 存放所有人的名称
    if len(face_names) == 0: # 如果还没有任何人脸数据，则直接退出
      return

    # 遍历 src_path 路径
    '''将 src_path 路径下的所有照片作为数据集'''
    i = 0
    j = 0
    count = [0] # 元素位数表示人脸采集总人数，元素表示对应人脸的样本数
    while(True):
      path_demo = self.src_path + '/' + face_names[i] + '/'+ face_names[i] + (str)(j) + '.jpg'
      if(os.path.exists(path_demo) is False): # 判断此人的人脸是否检测完
        i += 1
        if(i is len(face_names)): # 当所有人的脸都收集完后，退出循环
          break
        j = 0
        count.append(0)
        path_demo = self.src_path+'/'+face_names[i]+'/'+face_names[i]+(str)(j)+'.jpg'
      face = self.get_face_fromFile(path_demo) # 从路径获取一张图片
      if face is not None:
        faces.append(cv.cvtColor(face, cv.COLOR_BGR2GRAY)) # 取灰度图像
        count[i] +=1 # 计数
      j += 1
    print(face_names)
    print(count)

    # 由于人脸检测后返回的人脸图像大小不一致，需要统一大小，又都是矩形，可使用缩放的方式可确保图像不发生形变
    max = 0 # 存储最大特征值正方向图片的边长
    for face in faces:
      if(np.size(face[0]) > max):
        max = np.size(face[0])

    # 将小图像都进行放大
    for i in range(len(faces)):
      faces[i] = cv.resize(faces[i], (max,max), interpolation=cv.INTER_LANCZOS4)

    ''' 生成标签，按序表示每个人的序号 '''
    labels = np.empty(0, int)
    for i in range(len(count)):
      labels = np.append(labels, np.full(count[i], i, int))

    ''' 打乱数据集顺序 '''
    index = np.array([i for i in range(len(faces))]) # 创建一个内容为[0,1,2,...,len(y)-1]的列表
    np.random.seed(1) # 产生一组随机数序列
    np.random.shuffle(index) # 打乱 index 列表内容顺序，可以获得一个对 人脸特征图和标签 数组的乱序下标
    # # 获得打乱顺序的训练数据和训练标签，避免训练数据的顺序对模型的影响，增加模型的泛化能力。
    faces = np.array(faces)
    train_data = faces[index]
    train_label = labels[index]

    # 训练模型并输出
    self.recognizer.train(train_data, train_label)
    self.recognizer.write(fileName)
    print("[debug]: Successed output model file:" + fileName)


  '''
  从文件路径获取含有人脸的图像返回
  pct_path: picture path
  返回值:
      None   - 暂无识别人脸
      非None - 人脸图像（仅保留了人脸部分）
  '''
  # 从指定文件路径中获取所有图片，并将有人脸信息的图像存入列表返回
  def get_face_fromFile(self, pct_path):
    if os.path.isfile(pct_path) :
      img = cv.imread(pct_path)
      # 检测脸部
      ret, rect= self.isFace(img)
      # 注意：人脸检测后返回的图像大小不一致，需要统一大小(在主程序中处理)
      # 我们忽略未检测到的脸部
      if ret:
        return img[rect[1]:rect[1]+rect[3], rect[0]:rect[0]+rect[2]]
      else:
        print(pct_path + ' no found face')
        return None
    else:
      print(" no found" + pct_path)
    return None


  # 从摄像头获取人脸图片，并将有人脸信息的图像存入列表返回
  def get_face_fromCam(self):
    while(True): # 循环采集摄像头内图片，找到人脸
      _, img = self.cam.read()
      if(_ is False):
        continue
      ret, rect = self.isFace(img)
      # 注意：人脸检测后返回的图像大小不一致，需要统一大小(在主程序中处理)
      # 我们忽略未检测到的脸部
      if (ret) :
        return img



