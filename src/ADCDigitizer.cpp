#include "ADCDigitizer.h"

ADCDigitizer::ADCDigitizer() 
    : DigitizationBase("ADC") {
}

void ADCDigitizer::initializeTree() {
    // 创建Tree
    dataTree = std::make_unique<TTree>("adcEvents", "ADC Digitization Events");
    
    // 添加分支
    dataTree->Branch("inputEnergy", &inputEnergy, "inputEnergy/D");
    dataTree->Branch("peSiPM", &peSiPM, "peSiPM/D");
    dataTree->Branch("adcIni", &adcIni, "adcIni/I");
    dataTree->Branch("adcGainMean", &adcGainMean, "adcGainMean/D");
    dataTree->Branch("adcGainSigma", &adcGainSigma, "adcGainSigma/D");
    dataTree->Branch("adcGain", &adcGain, "adcGain/D");
    dataTree->Branch("gainRange", &gainRange, "gainRange/I");
    dataTree->Branch("outputEnergy", &outputEnergy, "outputEnergy/D");
}

void ADCDigitizer::initializeSamplingTree() {
    // 创建均匀抽样树
    samplingTree = std::make_unique<TTree>("adcSampling", "ADC Uniform Sampling Events");
    
    // 添加分支 - 与常规树相同的分支
    samplingTree->Branch("inputEnergy", &inputEnergy, "inputEnergy/D");
    samplingTree->Branch("peSiPM", &peSiPM, "peSiPM/D");
    samplingTree->Branch("adcIni", &adcIni, "adcIni/I");
    samplingTree->Branch("adcGainMean", &adcGainMean, "adcGainMean/D");
    samplingTree->Branch("adcGainSigma", &adcGainSigma, "adcGainSigma/D");
    samplingTree->Branch("adcGain", &adcGain, "adcGain/D");
    samplingTree->Branch("gainRange", &gainRange, "gainRange/I");
    samplingTree->Branch("outputEnergy", &outputEnergy, "outputEnergy/D");
}

double ADCDigitizer::digitize(double energy) {
    // 记录输入能量
    inputEnergy = energy;
    
    // 获取参数
    double CryLY = params.getParameter("EcalCryIntLY");
    double CryAtt = params.getParameter("EcalCryAtt");
    double SiPMPDE = params.getParameter("EcalSiPMPDE");
    double SiPMGainMean = params.getParameter("EcalSiPMGainMean");
    double FEENoiseSigma = params.getParameter("EcalFEENoiseSigma");
    double ASICNoiseSigma = params.getParameter("EcalASICNoiseSigma");
    double pedestal = params.getParameter("Pedestal");
    // double adcMax = params.getParameter("ADC");
    int ADCbit = params.getParameter("ADCbit");
    int adcMax = std::pow(2, ADCbit) - 1;
    int adcSwitch = params.getParameter("ADCSwitch");
    double gainRatio12 = params.getParameter("GainRatio_12");
    double gainRatio23 = params.getParameter("GainRatio_23");
    
    // 计算SiPM光电子数
    double nDet = energy * CryLY * CryAtt * SiPMPDE;
    peSiPM = nDet;
        
    // 计算ADC值
    double ADCMean = nDet * SiPMGainMean + pedestal;
    double ADCSigma = std::sqrt(FEENoiseSigma * FEENoiseSigma + ASICNoiseSigma * ASICNoiseSigma);
    int adc = std::round(rand.Gaus(ADCMean, ADCSigma));
    if (adc < 0) adc = 0;
    adcIni = adc;
    
    // 增益范围1（高增益）
    if (adc <= adcSwitch) {
        gainRange = 1;

        adcGainMean = ADCMean;
        adcGainSigma = ADCSigma;

        if(adc < 0) adc = 0;
        adcGain = adc;

        // 转换回能量
        outputEnergy = (adc - pedestal) / SiPMGainMean / (CryLY * CryAtt * SiPMPDE);
    }
    // 增益范围2（中增益）
    else if (adc > adcSwitch && static_cast<int>(adc / gainRatio12) <= adcSwitch) {
        gainRange = 2;
        
        // 调整增益和噪声
        double adjustedGain = SiPMGainMean / gainRatio12;
        double adjustedFEENoise = FEENoiseSigma / gainRatio12;
        
        // 重新计算ADC值
        ADCMean = nDet * adjustedGain + pedestal;
        ADCSigma = std::sqrt(ASICNoiseSigma * ASICNoiseSigma + adjustedFEENoise * adjustedFEENoise);
        adc = std::round(rand.Gaus(ADCMean, ADCSigma));

        adcGainMean = ADCMean;
        adcGainSigma = ADCSigma;
        
        // ADC截断
        if (adc < 0) adc = 0;
        adcGain = adc;

        // 转换回能量
        outputEnergy = (adc - pedestal) / adjustedGain / (CryLY * CryAtt * SiPMPDE);
    }
    // 增益范围3（低增益）
    else {
        gainRange = 3;
        
        // 调整增益和噪声
        double adjustedGain = SiPMGainMean / (gainRatio12 * gainRatio23);
        double adjustedFEENoise = FEENoiseSigma / (gainRatio12 * gainRatio23);
        
        // 重新计算ADC值
        ADCMean = nDet * adjustedGain + pedestal;
        ADCSigma = std::sqrt(ASICNoiseSigma * ASICNoiseSigma + adjustedFEENoise * adjustedFEENoise);
        adc = std::round(rand.Gaus(ADCMean, ADCSigma));

        adcGainMean = ADCMean;
        adcGainSigma = ADCSigma;
        
        // ADC截断
        if (adc < 0)
        {
            adc = 0;
        } 
        else if (adc > adcMax)
        {
            adc = adcMax;
        }
        adcGain = adc;

        // 转换回能量
        outputEnergy = (adc - pedestal) / adjustedGain / (CryLY * CryAtt * SiPMPDE);
    }
    
    // 填充Tree
    if (dataTree) dataTree->Fill();
    
    return outputEnergy;
} 