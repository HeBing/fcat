#!/usr/bin/env python
#----------------------------------------
# model stacking
# @NOTE: this script uses
#    a linear combination
#    to integrate results from 
#    base learner
# @NOTE: benchmark is automatically added
# @NOTE: train data's control is automaticall
#   extracted and used for inference
#-----------------------------------------
import shlex, subprocess, sys
import os, re, time
import collections
import createBenchmarkFeature as bm
import math
#---
# check args
#---
def menu(argv) :
  if len(argv) == 1 :
    print "/*----------------------------------------*/"
    print "/*              model stack               */"
    print "/*----------------------------------------*/"
    print "/* -model model1,model2,model3,...        */"
    print "/*     available models:                  */"
    print "/*       LogisticRegressionL1             */"
    print "/*       LogisticRegressionL2             */"
    print "/*       RandomForest                     */"
    print "/* -train trainFile: train file           */"
    print "/* -test testFile: test file              */"
    print "/* -output outputFile: output file        */"
    print "/*----------------------------------------*/"
    return -1

  idx = 1 # argv[0] is the script name
  modelOK = 0; trainOK = 0; testOK = 0; outOK = 0;

  model = []
  trainFile = None; testFile = None; outputFile = None; k = 1

  while idx < len(argv) :
    if sys.argv[idx] == '-model' :
      idx += 1
      model = sys.argv[idx].split(',')
      modelOK = 1
    elif sys.argv[idx] == '-train' :
      idx += 1
      trainFile = sys.argv[idx]
      trainOK = 1
    elif sys.argv[idx] == '-test' :
      idx += 1
      testFile = sys.argv[idx]
      testOK = 1
    elif sys.argv[idx] == '-output' :
      idx += 1
      outputFile = sys.argv[idx]
      outOK = 1
    else : 
      print "Unknow parameters", sys.argv[idx]
      return -1
    idx += 1

  if modelOK+trainOK+testOK+outOK < 4 :
    print "Missing one parameter! See below: "
    print "/*----------------------------------------*/"
    print "/*              model stack               */"
    print "/*----------------------------------------*/"
    print "/* -model model1,model2,model3,...        */"
    print "/*     available models:                  */"
    print "/*       LogisticRegressionL1             */"
    print "/*       LogisticRegressionL2             */"
    print "/*       RandomForest                     */"
    print "/* -train trainFile: train file           */"
    print "/* -test testFile: test file              */"
    print "/* -output outputFile: output file        */"
    print "/*----------------------------------------*/"
    return -1
  return (model,trainFile,testFile,outputFile,k)

  
  
#-----
# functions for cross validation
#---

def splitFileCV(file, fold):
  # for train files, split it into fold training files and testing files
  # in total 4 pairs of training and testing files.
  # use name convention to communicate file names
  # file_cv1_test, file_cv1_train
  # file_cv2_test, file_cv2_train
  # default output dir is the dir where file is in
  f = open(file, 'r')
  lines = f.readlines()
  f.close()

  for i in range(fold):
    # print '- spliting for fold %d' % i
    if os.path.isfile(file + '_cv' + repr(i+1) + '_test') :
      continue
    f1 = open(file + '_cv' + repr(i+1) + '_test', 'w')
    f1.write(''.join( lines[i::fold] ))
    f1.close()

    f2 = open(file + '_cv' + repr(i+1) + '_train', 'w')
    lines2 = lines[:] # copy the list, instead of copying the ref
    for l in lines[i::fold] :
      lines2.remove(l)
    f2.write(''.join( lines2 ))
    f2.close()

  return 0
 
def crossValidated(model, file, fold): 

  for k in range(fold) :
    trainFileTmp = file+'_cv'+str(k+1)+'_train'
    testFileTmp = file+'_cv'+str(k+1)+'_test'
    addBenchmark(trainFileTmp, 1)
    addBenchmark(testFileTmp, 1);

  for mymodel in model:
    for k in range(fold):
      trainFileTmp = file+'_cv'+str(k+1)+'_train'
      testFileTmp = file+'_cv'+str(k+1)+'_test'
      print '- training %s using %s' % (mymodel, trainFileTmp)
      tmpResultFile = '%s_trained%s' % (trainFileTmp, mymodel)
      if os.path.isfile(tmpResultFile) :
	      continue
      if mymodel == 'Benchmark' : 
        cmd = './trainModel -m LogisticRegressionL1 -c 10000 -t %s -o %s_trained%s' \
        % (trainFileTmp+'_Benchmark', trainFileTmp, mymodel)
      elif mymodel == 'RandomForest' :
        cmd = './trainModel -m %s -c 1 -t %s -o %s_trained%s' % \
          (mymodel, trainFileTmp, trainFileTmp, mymodel)
      else : # liblinear model
        (modelname, modelc) = mymodel.split('_')
        cmd = './trainModel -m %s -c %s -t %s -o %s_trained%s' %  \
          (modelname, modelc, trainFileTmp, trainFileTmp, mymodel)

      # train models
      myargs = shlex.split(cmd);
      start = time.time()
      p = subprocess.Popen(myargs, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
      for x in p.stdout.read().split('\n') :
        if re.search(r"Accuracy|nonzero", x) :
          continue
      for x in p.stderr.read().split('\n') :
        if re.search(r"Accuracy|core", x) :
          continue
      elapsed = time.time() - start

  # make prediction
  for k in range(fold):
    trainFileTmp = file+'_cv'+str(k+1)+'_train'
    testFileTmp = file+'_cv'+str(k+1)+'_test'
    predictModels(trainFileTmp, testFileTmp,  model)
	
  # read result for eaech cv
  errs = []
  for k in range(fold):
    trainFileTmp = file+'_cv'+str(k+1)+'_train'
    testFileTmp = file+'_cv'+str(k+1)+'_test'
    result = []
    # first read ys
    p = subprocess.Popen(shlex.split('cut -f 1 -d\  %s' % testFileTmp),stdout=subprocess.PIPE)
    ys = [ int(a) for a in p.stdout.read().split('\n') if len(a) > 0 ] # last element is ''

    for mymodel in model:
      tmpResultFile = '%s_result%sBy%s' % (testFileTmp, mymodel, os.path.basename(trainFileTmp)[:4])
      fp = open(tmpResultFile, "r")
      if re.match(r"LogisticRegression|Benchmark", mymodel) :
        tmp = fp.read().splitlines()
        labels = tmp[0].split(' ')
        if labels[1] == '1':
          labelIdx = 1
        if labels[2] == '1':
          labelIdx = 2
        result.append([ float(x.split(' ')[labelIdx]) for x in tmp[1:] ])
      else :
        result.append([ float(x) for x in fp.read().splitlines()] )
      fp.close()
 
    errs.append([ err(yhat, ys) for yhat in result ])

  precisions = []
  for i in range(len(errs[0])):
    finalErrs = 0
    for k in range(len(errs)):
      finalErrs = finalErrs + errs[k][i]
    precisions.append(1/finalErrs);

  weights = [ a/sum(precisions) for a in precisions ]

  return weights

#---
# add benchmark automatically
#---
def addBenchmark(featureFile, k) :
  bmFeatureFile = ''.join([featureFile,'_Benchmark'])
  if os.path.isfile(bmFeatureFile) == False :
    bmFeatureFile = bm.createBenchmarkFeature(featureFile, k)
  if bmFeatureFile < 0:
    print 'Error when creating benchmark features for %s' % featureFile
    return -1
  return 0

#---
# addDiffPenalty
#---
def addDiffPenalty(model,cs) :
  tmpModel = []
  rmModel = []
  for m in model :
    if m != 'RandomForest' and m != 'Benchmark' : # rf is the only model without c
      tmpModel += [ m+'_'+repr(c) for c in cs ]
      rmModel.append(m)

  for a in rmModel :
    model.remove(a) # remove

  model.extend(tmpModel)

  return 0

#---
# trainModels
#---
def trainModels(trainFile, testFile, model) :
  for mymodel in model :
    print '- training %s using %s' % (mymodel, trainFile)

    if os.path.isfile('%s_trained%s' % (trainFile, mymodel)) :
      continue
    if mymodel == 'Benchmark' :
      cmd = './trainModel -m LogisticRegressionL1 -c 10000 -t %s -o %s_trained%s' \
        % (trainFile+'_Benchmark', trainFile, mymodel)
    elif mymodel == 'RandomForest' :
      cmd = './trainModel -m %s -c 1 -t %s -o %s_trained%s' % \
        (mymodel, trainFile, trainFile, mymodel)
    else : # model with c value added
      (modelname, modelc) = mymodel.split('_')
      # print '-- with c = %s' % modelc
      cmd = './trainModel -m %s -c %s -t %s -o %s_trained%s' %  \
        (modelname, modelc, trainFile, trainFile, mymodel)

    # run model
    myargs = shlex.split(cmd);
    start = time.time()
    p = subprocess.Popen(myargs, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
    for x in p.stdout.read().split('\n') :
      if re.search(r"Accuracy|nonzero", x) :
        continue
    for x in p.stderr.read().split('\n') :
       if re.search(r"Accuracy|core", x) :
         continue
    elapsed = time.time() - start
    # print 'elapsed time: %.8f' % elapsed

#---
# predictModels
#---
def predictModels(trainFile, testFile, model):
  for mymodel in model :
    print '- predicting %s using %s' % (testFile, mymodel)
    if os.path.isfile('%s_result%sBy%s' % (testFile,mymodel, os.path.basename(trainFile)[:4])) :
      continue
    if mymodel == 'Benchmark':
      cmd = './predictModel -m LogisticRegressionL1 -tm %s_trained%s -train %s -test %s -o %s_result%sBy%s' \
        % (trainFile, mymodel, trainFile+'_Benchmark', testFile+'_Benchmark', testFile, \
            mymodel, os.path.basename(trainFile)[:4])
    else :
      modelname = mymodel.split('_')[0]
      cmd = './predictModel -m %s -tm %s_trained%s -train %s -test %s -o %s_result%sBy%s' \
        % (modelname, trainFile, mymodel, trainFile, testFile, \
          testFile, mymodel, os.path.basename(trainFile)[:4])

    # print '- cmd is %s' % cmd

    # run predict model
    myargs = shlex.split(cmd)
    start = time.time()
    p = subprocess.Popen(myargs, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
    for x in p.stdout.read().split('\n') :
      if re.search(r"Accuracy", x) :
        continue
    for x in p.stderr.read().split('\n') :
      if re.search(r"Accuracy|core", x) :
        continue
    elapsed = time.time() - start

#---
# collectResults
#---
def collectResults(trainFile, testFile, model) :
  

  # read ys
  p = subprocess.Popen(shlex.split('cut -f 1 -d\  %s' % testFile),stdout=subprocess.PIPE)
  ys = [ int(a) for a in p.stdout.read().split('\n') if len(a) > 0 ] # last element is ''
  
  result = []
  # read result
  for mymodel in model :
    tmpResultFile = '%s_result%sBy%s' % (testFile, mymodel, os.path.basename(trainFile)[:4])
    file = open(tmpResultFile, "r")
    if re.match(r"LogisticRegression|Benchmark", mymodel) :
      tmp = file.read().splitlines()
      labels = tmp[0].split(' ')
      if labels[1] == '1':
        labelIdx = 1
      if labels[2] == '1':
        labelIdx = 2
      result.append([ float(x.split(' ')[labelIdx]) for x in tmp[1:] ])
    else :
      result.append([ float(x) for x in file.read().splitlines()] )
    file.close()
  
  return (ys, result)

#---
# err func
#    low-level
#---
def err(prediction, truth) :
  if len(prediction) != len(truth) :
    print "length of prediction and truth are not the same\n"
    print 'len of prediction is %d and len of truth is %d' % (len(prediction),len(truth))
    return -1
  err = 0.0
  for i in range(len(prediction)) :
    err += (float(prediction[i]) - float(truth[i]))**2 # mse
  err = err/(len(prediction)-1)
  return err

#---
# getVotingWeights
#---
def getVotingWeights(model, file, fold = 3) :
  print '-------------------------------------'
  print ' calculating weights '
  splitFileCV(file, fold)
  weights = crossValidated(model, file, fold)
  return weights

#---
# getVotingResults
#---
def getVotingResults(result, weights, top) :
  votedResult = []
  if len(result) != len(weights) :
    print "len of result is not equal to len of weights"
    return -1
  topWeights = sorted(weights, reverse = True)[:top]
  for i in range(len(result[0])) :
    tmp = 0.0
    for j in range(len(weights)) :
      if weights[j] in topWeights :
        tmp += float(result[j][i])*(weights[j]/sum(topWeights))
    votedResult.append(tmp)

  return votedResult

#---
# writeFormatted
# format: 1 1:0.123 2:1.234 ..
#--
def writeFormatted(ys, result, outputFile) :
  fp = open(outputFile, 'w')
  for i in range(len(ys)) :
    fp.write(repr(ys[i])+' ')
    for j in range(len(result)) :
      if j == len(result) - 1 :
        fp.write(''.join( [repr(j+1),':', repr(result[j][i]), '\n'] ))
      else :
        fp.write(''.join( [repr(j+1),':', repr(result[j][i]), ' '] ))
  return 0

#---
# reserveControl
#---
def reserveControl(trainFile, m) :
  if os.path.isfile(trainFile + '_train'):
    return 0
  f = open(trainFile, 'r')
  f_train = open(trainFile + '_train', 'w')
  f_ctrl = open(trainFile + '_control', 'w')
  ctrl = 0
  idx = 0
  for line in f :
    if ctrl == 0:
      f_train.write(line)
      if idx == 0 and re.match(r"0 ", line) :
        ctrl = 1
    else :
      f_ctrl.write(line)
      idx += 1
      if idx > m :
        ctrl = 0
  f_train.close()
  f_ctrl.close()
  return 0


#----------------------------------
#  AUC
#-----------------------------------
def calcAUC(y,x) :
  if len(x) != len(y):
    print 'x y lens differ!'
    return -1
  area = 0.0
  for i in range(1,len(y)):
    area += 1.0/2.0*(y[i-1]+y[i])*(x[i]-x[i-1])*(-1)
  return area


def uniqueValueIdx(a):
  d = collections.defaultdict(list)
  for i,j in enumerate(a):
    d[j].append(i)
  return d

def doInference(Y, pred, ctrl):
  sens = []

  FPR = []
  estFDR = []
  trueFDR = []

  auc = []

  all = len(Y)
  allP = sum(Y)
  allN = all - allP

  predY = sorted(zip(pred, Y))
  predSorted = [ a for (a,b) in predY ]
  YSorted = [ b for (a,b) in predY ]

  uniquePred = sorted(list(set(pred)))
  uniquePredIdx = uniqueValueIdx(predSorted)
  uniquePvalue = [ calcPvalue(a,ctrl) for a in uniquePred ]   

  # inference
  for i in range(len(uniquePred)) :

    tmpIdx = uniquePredIdx[uniquePred[i]][0]
    tmpY = YSorted[tmpIdx:]
    sumTmpY = sum(tmpY)
    lenTmpY = len(tmpY)

    # sens
    sens.append( float(sumTmpY) / float(allP) )
    # FPR
    FPR.append( float((lenTmpY-sumTmpY)) / float(allN) )
    # true FDR
    trueFDR.append( float(lenTmpY-sumTmpY) / float(len(tmpY)) )
    # est FDR
    tmpEstFDR = uniquePvalue[i] * float(all) / ( float(all - tmpIdx) )
    if i == 0: 
      tmpMin = tmpEstFDR
    elif tmpEstFDR < tmpMin :
      tmpMin = tmpEstFDR
    estFDR.append( min(tmpMin, 1.0) )

  sens.append( 0.0 )
  FPR.append( 0.0 )
  trueFDR.append( 0.0 )
  estFDR.append( 0.0 )

  # make sure true FDR is decreasing
  for i in range(1,len(trueFDR)) :
    if trueFDR[i-1] < trueFDR[i] :
      trueFDR[i] = trueFDR[i-1]

  # adjustPvalue has a problem, rank (k) is discrete
  uniqueEstFDR = uniqueValueIdx(uniquePvalue)


  # return
  return (sens, FPR, estFDR, trueFDR)

def calcPvalue(value, ctrl) :
  return float(sum(( int(a >= value) for a in ctrl )))/float(len(ctrl))

#---
# main
#---
def main(argv) :

  if menu(argv) != -1:
    (model, trainFile, testFile, outputFile, k) = menu(argv)
  else :
    sys.exit(-1)
  


  # reserve control for inference
  m = 500
  reserveControl(trainFile, m)

  # add benchmark
  model.append('Benchmark')
  addBenchmark(trainFile + '_train', k)
  addBenchmark(trainFile + '_control', k)
  addBenchmark(testFile, k)

  # add penalty for penalty-based method
  cs = [0.001, 0.01]
  addDiffPenalty(model, cs)

  print '---------------------------------------------'
  print '- training models '
  trainModels(trainFile + '_train', testFile,  model)

  # predict model
  print '----------------------------------------'
  print '- predicting with models '
  predictModels(trainFile + '_train', trainFile + '_train', model) 
  predictModels(trainFile + '_train', trainFile + '_control', model) 
  predictModels(trainFile + '_train', testFile, model)

  # collect results
  start = time.time()
  (trainYs, trainResult) = collectResults(trainFile + '_train', \
      trainFile + '_train', model) 
  elapsed = time.time() - start

  ## collect control
  start = time.time()
  (controlYs, controlResult) = collectResults(trainFile + '_train', \
      trainFile + '_control', model) 
  elapsed = time.time() - start

  ## collect test
  start = time.time()
  (testYs, testResult) = collectResults(trainFile + '_train', \
      testFile, model) 
  elapsed = time.time() - start

  weights = getVotingWeights(model, trainFile, fold = 3)
  top = 2
  votedResult = getVotingResults(testResult, weights, top)
  votedResultControl = getVotingResults(controlResult, weights, top)

  # write result
  testResult.append(votedResult)
  controlResult.append(votedResultControl)

  model.append('voting')

  tmpmodel = list(set([ a.split('_')[0] for a in model]))
  outputFile = ''.join( [ trainFile, \
      os.path.basename(testFile),','.join(tmpmodel), '_result'] )

  writeFormatted(testYs, testResult, outputFile)
  
  # conduct inference
  start = time.time()

  ## ROC
  testROC = []
  testAUC = []
  ## est FDR
  estFDR = []
  estFDRAUC = []
  ## true FDR
  trueFDR = []
  trueFDRAUC = []

  print '----------------------------------------------'
  print '- doing inference '
  for i in range(len(testResult)) :

    (sensi, FPRi, estFDRi, trueFDRi) = doInference(testYs, testResult[i], controlResult[i])

    # test results ROC, plot1
    testAUC.append(calcAUC(sensi,FPRi))
    testROC.append(sensi)
    testROC.append(FPRi)

    # test results, estFDR, plot2
    estFDRAUC.append(calcAUC(sensi,estFDRi))
    estFDR.append(sensi)
    estFDR.append(estFDRi)

    # test results, trueFDR, plot 3
    trueFDRAUC.append(calcAUC(sensi,trueFDRi))
    trueFDR.append(sensi)
    trueFDR.append(trueFDRi)

  elapsed = time.time() - start

  # write ROC
  print '---------------------------------------------'
  print '- writing to output '
  print '- ', outputFile
  print '---------------------------------------------'
  outputFileROC = ''.join( [ trainFile, \
      os.path.basename(testFile),','.join(tmpmodel), '_rocResult'] )

  f = open(outputFileROC, 'w')

  lenROC = [ len(a) for a in testROC ]

  for j in range( max(lenROC) ):
    for i in range(len(testROC)):
      if i == len(testROC) - 1:
        try: 
          f.write(repr(testROC[i][j])+'\n')
        except IndexError:
          f.write('NA'+'\n')
      else:
        try: 
          f.write(repr(testROC[i][j])+' ')
        except IndexError:
          f.write('NA'+' ')
  f.close()

  # write est FDR
  outputFileEstFDR = ''.join( [ trainFile, \
      os.path.basename(testFile),','.join(tmpmodel), '_estFDRResult'] )

  f = open(outputFileEstFDR, 'w')

  lenEstFDR = [ len(a) for a in estFDR ]

  for j in range( max(lenEstFDR) ):
    for i in range(len(estFDR)):
      if i == len(estFDR) - 1:
        try:
          f.write(repr(estFDR[i][j])+'\n')
        except IndexError:
          f.write('NA'+'\n')
      else:
        try:
          f.write(repr(estFDR[i][j])+' ')
        except IndexError:
          f.write('NA'+' ')

  f.close()

  # write true FDR
  outputFileTrueFDR = ''.join( [ trainFile, \
      os.path.basename(testFile),','.join(tmpmodel), '_trueFDRResult'] )

  f = open(outputFileTrueFDR, 'w')

  lenTrueFDR = [ len(a) for a in trueFDR ]

  for j in range( max(lenTrueFDR) ):
    for i in range(len(trueFDR)):
      if i == len(trueFDR) - 1:
        try:
          f.write(repr(trueFDR[i][j])+'\n')
        except IndexError:
          f.write('NA'+'\n')
      else:
        try:
          f.write(repr(trueFDR[i][j])+' ')
        except IndexError:
          f.write('NA'+' ')

  f.close()

  # write AUC
  outputFileAUC = ''.join( [ trainFile, \
      os.path.basename(testFile),','.join(tmpmodel), '_AUC'] )
  f = open(outputFileAUC, 'w')
  f.write(' '.join(model))
  f.write('\n')
  f.write(' '.join([ repr(a) for a in testAUC ]))
  f.write('\n')
  f.write(' '.join([ repr(a) for a in estFDRAUC ]))
  f.write('\n')
  f.write(' '.join([ repr(a) for a in trueFDRAUC ]))
  f.write('\n')
 
  f.close()
  
  return 0

if __name__ == '__main__':
  # added to set up environment path for boost lib
  if 'LD_LIBRARY_PATH' not in os.environ:
    os.environ['LD_LIBRARY_PATH']  = (":" + '../rt-rank_1.5/cart/boost/1.55.0/lib')
    os.execve(os.path.realpath(__file__), sys.argv, os.environ)
  elif '../rt-rank_1.5/cart/boost/1.55.0/lib' not in os.environ['LD_LIBRARY_PATH']:
    os.environ['LD_LIBRARY_PATH']  += (":" + '../rt-rank_1.5/cart/boost/1.55.0/lib')
    os.execve(os.path.realpath(__file__), sys.argv, os.environ)
  main(sys.argv)
