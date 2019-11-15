import numpy as np
import argparse
# i need to define a class, yeah, k-means clustering
# yuqianjin. 2019.3.07

class faceClassfy:
    def __init__(self, num_k_cluster, dimensions, num_samples, allSampleData,
                 one_cluster_min, one_cluster_max, threshold, model_file_name, iteration):
        self.k = num_k_cluster
        self.dimensions = dimensions
        self.samples = num_samples
        self.data = allSampleData # np array
        self.centroid = np.array((self.k, self.dimensions))
        self.clusterMin = one_cluster_min
        self.clusterMax = one_cluster_max
        self.cosDistanceThreshold = threshold
        self.model_file = model_file_name
        self.iters = iteration
        self.adoptClusterIndex = {}

    def computeCosDistance(left, right):
        topValue = np.multiply(left, right)
        bottomleftValue = np.sum(np.square(left))
        bottomRightValue = np.sum(np.square(right))
        return topValue/(bottomleftValue+bottomRightValue)

    def computeEulDistance(left, right):
        subtractValue = np.subtract(left, right)
        return np.sqrt(np.sum(np.square(subtractValue)))

    # this function need init k centeroid with dimension values, one method is random k sample from total samples
    # and other method is rectfiled to improve k-means clustering strong-ability, so this details is as follows:
    # first, we should give this first centroid value, total sample mean value, second, that is
    def init_centroid(self):
        tempData  = np.copy(self.data)
        for ii in range(self.k):
            self.adoptClusterIndex[ii] = []
            centroid = np.array((self.k, self.dimensions))
            centroid[ii] = float(np.mean(tempData, axis=0))
            for ii in range(tempData.shape[0]):
                cosValue = self.computeCosDistance(self.centroid[0], tempData[ii])
                if cosValue > self.cosDistanceThreshold:
                    np.delete(tempData, ii, axis=0)

# i dont know how to define array values
    def faceTrain(self):
        for iter in range(self.iters):
            # 划分样本到各个类当中
            for ii in range(self.samples):
                tempValue = 0.0
                tempIndex = 0
                for jj in range(self.k):
                    cosValue = self.computeCosDistance(self.centroid[jj], self.data[ii])
                    eulValue = self.computeEulDistance(self.centroid[jj], self.data[ii]) # ?????
                    if tempValue > cosValue:
                        tempValue = cosValue
                        tempIndex = jj
                self.adoptClusterIndex[tempIndex].append(ii)
            # 统计各个类的数目
            for jj in range(self.k):
                # 少于规定的情况下， 将该类划分到其他类当中
                if len(self.adoptClusterIndex[jj]) < self.clusterMin:
                    self.k = self.k -1
                    np.delete(self.centroid, jj, axis= 0)
                    for ii in range(len(self.adoptClusterIndex[jj])):
                        tempValue = 0.0
                        tempIndex = 0
                        for mm in range(self.k):
                            cosValue = self.computeCosDistance(self.centroid[mm], self.data[self.adoptClusterIndex[jj][ii]])
                            eulValue = self.computeEulDistance(self.centroid[mm], self.data[self.adoptClusterIndex[jj][ii]])  # ?????
                            if tempValue > cosValue:
                                tempValue = cosValue
                                if mm == jj:
                                    tempIndex = mm + 1
                                else:
                                    tempIndex = mm
                        self.adoptClusterIndex[tempIndex].append(ii)
                # 多于该类的情况下，将多余的重新划分出一类
                if len(self.adoptClusterIndex[jj]) > self.clusterMax:
                    self.adoptClusterIndex[self.k] = []
                    for ii in range(len(self.adoptClusterIndex[jj])):
                        for mm in range(self.k):
                            cosValue = self.computeCosDistance(self.centroid[mm], self.data[self.adoptClusterIndex[jj][ii]])
                            eulValue = self.computeEulDistance(self.centroid[mm], self.data[self.adoptClusterIndex[jj][ii]])  # ?????
                            if tempValue < self.cosDistanceThreshold:
                                self.adoptClusterIndex[jj].pop(self.data[self.adoptClusterIndex[jj][ii]])
                                self.adoptClusterIndex[self.k].append(self.data[self.adoptClusterIndex[jj][ii]])
                    self.k += 1
            print ("k_means for k:", self.k)
             # 更新权重
            for jj in range(self.k):
                sumConditators = np.array(self.dimensions)
                for ii in range(len(self.adoptClusterIndex[jj])):
                    sumConditators += self.data[self.adoptClusterIndex[jj][ii]]
                self.centroid[jj] = float(sumConditators/len(self.adoptClusterIndex[jj]))
        # 保存 模型文件
        if iter ==self.iters-1:
            save_file = open(self.model_file, "w")
            for line in range(self.k):
                save_file.write(self.centroid[line]+"\n")

    def faceTest(self, testData, threshold):
        tempValue = 0.0
        tempIndex = 0
        for ii in range(len(self.centroid)):
            cosValue = self.computeCosDistance(self.centroid[ii], testData)
            eulValue = self.computeEulDistance(self.centroid[ii], testData) # ??????
            if tempValue > cosValue:
                tempValue = cosValue
                tempIndex = ii
        return tempIndex, tempValue


def makerParseAugment():
    parser = argparse.ArgumentParser()
    parser.add_argument('--datafile', type=str, required=True, help='.txt file for inference face feature')
    parser.add_argument('-k', type=int,required=True, help='k-means clusters classfly for the k number')
    parser.add_argument('-num', type=int, required=True, help='sample numbers')
    parser.add_argument('-dimension', type=int, required=True, help='face dimension')
    parser.add_argument('--modelsavefile', type=str, required=True, help='.model file for save inference')
    return parser

def loadSampleData():
    augParse = makerParseAugment()
    args = augParse.parse_args()
    dataFile = args.datafile
    k_cluster = args.k
    savefile = args.modelsavefile
    numSample = args.num
    dim = args.dimension
    sampleData = np.array((numSample, dim))
    with open(dataFile, 'r') as data_file:
        count = 0
        while True:
            singleSampleData = data_file.readline().split(" ")
            sampleData[count] = np.array(singleSampleData)
            count +=1
    return sampleData, dim, numSample, k_cluster, savefile


def main():
    sampleData, dim, numSample, k_cluster, savefile = loadSampleData()
    oneClustermin = 80
    oneClustermax = 120
    clusterThreshold = 0.58 # ?????
    iteration = 2000 # ?????
    classflyFace = faceClassfy(k_cluster, dim, numSample, sampleData,
                 oneClustermin, oneClustermax, clusterThreshold,
                               savefile, iteration)
    classflyFace.faceTrain()


if __name__ =="_main_":
    main()

