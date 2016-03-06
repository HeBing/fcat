#!/bin/Rscript
args <- commandArgs(TRUE)

print(args)

outputFileROC = args[1]
outputFileEstFDR = args[2]
outputFileTrueFDR = args[3]
models = strsplit(x = args[4],split=",")[[1]]
top = as.numeric(args[5])

# library(RColorBrewer)
# cols = brewer.pal(length(models),"Set1")
cols = rainbow(length(models))
png(file = paste0(outputFileROC, ".png"), type = "cairo", bg = "transparent",
	width = 480*1.2*2, height = 480*1.2)
par(mfrow = c(1,2))

## plot 1 ROC
dat = read.table(outputFileROC)
plot(NA, xlim=c(0,1),ylim=c(0,1), xlab = "FPR", ylab = "TPR", 
	main = paste0(substr(outputFileROC,1,floor(nchar(outputFileROC)/2)),"\n",substr(outputFileROC,floor(nchar(outputFileROC)/2)+1,nchar(outputFileROC))), cex.main = 1.5)

for(i in 1:(ncol(dat)/2)) {
	if(models[i] == "voting" || models[i] == "Benchmark" || models[i] == "RandomForest") {
		mylty = 1
	}
	else {
		mylty = 2
	}

	lines(dat[,(i-1)*2+1]~dat[,i*2], col = cols[i], lty = mylty)
}

legend("bottomright", legend=c(models[-length(models)], paste0(models[length(models)],'Top',top)), col = cols, lty=1, lwd=2, cex = 1.5)


## plot 2 TPR v.s. estFDR
dat = read.table(outputFileEstFDR)
plot(NA, xlim=c(0,1),ylim=c(0,1), xlab = "est FDR", ylab = "TPR",
  main = paste0(substr(outputFileEstFDR,1,floor(nchar(outputFileROC)/2)),"\n",substr(outputFileEstFDR,floor(nchar(outputFileROC)/2)+1,nchar(outputFileEstFDR))), cex.main = 1.5)

for(i in 1:(ncol(dat)/2)) {

  if(models[i] == "voting" || models[i] == "Benchmark" || models[i] == "RandomForest") {
    mylty = 1
  }
  else {
    mylty = 2
  }
  
  lines(dat[,(i-1)*2+1]~dat[,i*2], col = cols[i], lty = mylty)

}

legend("bottomright", legend=c(models[-length(models)], paste0(models[length(models)],'Top',top)), col = cols, lty=1, lwd=2, cex = 1.5)


if(FALSE){
## plot 3 TPR v.s. trueFDR
## process FDR fun
dat = read.table(outputFileTrueFDR)
plot(NA, xlim=c(0,1),ylim=c(0,1), xlab = "true FDR", ylab = "TPR",
  main = paste0(substr(outputFileTrueFDR,1,floor(nchar(outputFileROC)/2)),"\n",substr(outputFileTrueFDR,floor(nchar(outputFileROC)/2)+1,nchar(outputFileTrueFDR))), cex.main=1.5)

for(i in 1:(ncol(dat)/2)) {
  if(models[i] == "voting" || models[i] == "Benchmark" || models[i] == "RandomForest") {
    mylty = 1
  }
  else {
    mylty = 2
  }

  lines(dat[,(i-1)*2+1]~dat[,i*2], col = cols[i], lty = mylty)
 
}

legend("bottomright", legend=c(models[-length(models)], paste0(models[length(models)],'Top',top)), col = cols, lty=1, lwd=2, cex=1.5)
}

dev.off()

