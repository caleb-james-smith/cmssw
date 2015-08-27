// -*- C++ -*-
//
// Package:    H2TestBeamAnalyzer
// Class:      H2TestBeamAnalyzer
// 
/**\class H2TestBeamAnalyzer H2TestBeamAnalyzer.cc UserCode/H2TestBeamAnalyzer/src/H2TestBeamAnalyzer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Authors:  Jay Dittmann, Nadja Strobbe, Joe Pastika
// Based on work by:  Viktor Khristenko,510 1-004,+41227672815,
//         Created:   Tue Sep 16 15:47:09 CEST 2014
// $Id$
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "EventFilter/HcalRawToDigi/interface/HcalHTRData.h"
#include "EventFilter/HcalRawToDigi/interface/HcalDCCHeader.h"
#include "EventFilter/HcalRawToDigi/interface/HcalUnpacker.h"
#include "DataFormats/HcalDetId/interface/HcalOtherDetId.h"
#include "DataFormats/HcalDigi/interface/HcalQIESample.h"
#include "DataFormats/HcalDetId/interface/HcalSubdetector.h"
#include "DataFormats/HcalDetId/interface/HcalCalibDetId.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/FEDRawData/interface/FEDHeader.h"
#include "DataFormats/FEDRawData/interface/FEDTrailer.h"
#include "DataFormats/FEDRawData/interface/FEDNumbering.h"
#include "DataFormats/FEDRawData/interface/FEDRawData.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "CalibFormats/HcalObjects/interface/HcalDbService.h"
#include "CalibFormats/HcalObjects/interface/HcalDbRecord.h"
#include "CalibFormats/HcalObjects/interface/HcalCalibrations.h"
#include "CalibFormats/HcalObjects/interface/HcalCoderDb.h"

#include "TBDataFormats/HcalTBObjects/interface/HcalTBTriggerData.h"
#include "TBDataFormats/HcalTBObjects/interface/HcalTBBeamCounters.h"
#include "TBDataFormats/HcalTBObjects/interface/HcalTBEventPosition.h"
#include "TBDataFormats/HcalTBObjects/interface/HcalTBParticleId.h"
#include "TBDataFormats/HcalTBObjects/interface/HcalTBTiming.h"

#include "RecoTBCalo/HcalTBObjectUnpacker/interface/HcalTBTriggerDataUnpacker.h"
#include "RecoTBCalo/HcalTBObjectUnpacker/interface/HcalTBSlowDataUnpacker.h"

#include "ADC_Conversion.h"

#include "TH1D.h"
#include "TH2D.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TProfile.h"
#include "TFile.h"
#include "TSystem.h"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

#define NUMCHS 300 
#define NUMTS 50

#define NUMADCS 128

double adc2fC[NUMADCS]={
    -0.5,0.5,1.5,2.5,3.5,4.5,5.5,6.5,7.5,8.5,9.5, 10.5,11.5,12.5,
    13.5,15.,17.,19.,21.,23.,25.,27.,29.5,32.5,35.5,38.5,42.,46.,50.,54.5,59.5,
    64.5,59.5,64.5,69.5,74.5,79.5,84.5,89.5,94.5,99.5,104.5,109.5,114.5,119.5,
    124.5,129.5,137.,147.,157.,167.,177.,187.,197.,209.5,224.5,239.5,254.5,272.,
    292.,312.,334.5,359.5,384.5,359.5,384.5,409.5,434.5,459.5,484.5,509.5,534.5,
    559.5,584.5,609.5,634.5,659.5,684.5,709.5,747.,797.,847.,897.,947.,997.,
    1047.,1109.5,1184.5,1259.5,1334.5,1422.,1522.,1622.,1734.5,1859.5,1984.5,
    1859.5,1984.5,2109.5,2234.5,2359.5,2484.5,2609.5,2734.5,2859.5,2984.5,
    3109.5,3234.5,3359.5,3484.5,3609.5,3797.,4047.,4297.,4547.,4797.,5047.,
    5297.,5609.5,5984.5,6359.5,6734.5,7172.,7672.,8172.,8734.5,9359.5,9984.5};

struct TCalibLedInfo
{
    int numChs;
    int iphi[50];
    int ieta[50];
    int cBoxChannel[50];
    vector<string> cBoxString;
    int nTS[50];
    double pulse[50][10];
};

struct TQIE8Info
{
    int numChs;
    int numTS;
    int iphi[NUMCHS];
    int ieta[NUMCHS];
    int depth[NUMCHS];
    double pulse[NUMCHS][NUMTS];
    double pulse_adc[NUMCHS][NUMTS];
    double ped[NUMCHS];
    double ped_adc[NUMCHS];
    bool valid[NUMCHS];
};

struct TQIE11Info
{
    int numChs;
    int numTS;
    int iphi[NUMCHS];
    int ieta[NUMCHS];
    int depth[NUMCHS];
    double pulse[NUMCHS][NUMTS];
    double ped[NUMCHS];
    double pulse_adc[NUMCHS][NUMTS];
    double ped_adc[NUMCHS];
    bool capid_error[NUMCHS];
    bool link_error[NUMCHS];
    bool soi[NUMCHS][NUMTS];
};




struct H2Triggers
{
    //
    //  Standard Triggers
    //
    int ped;
    int led;
    int laser;
    int beam;
    string str;

    //
    //  Added for completeness
    //
    int fakeTrg;
    int inSpillTrg;
};

struct H2BeamCounters
{
    double cer1adc;
    double cer2adc;
    double cer3adc;
    double s1adc;
    double s2adc;
    double s3adc;
    double s4adc;
};

struct H2Timing
{
    int s1Count;
    int s2Count;
    int s3Count;
    int s4Count;

    double triggerTime;
    double ttcL1Atime;
};

//
// class declaration
//

class H2TestBeamAnalyzer : public edm::EDAnalyzer 
{
public:
    explicit H2TestBeamAnalyzer(const edm::ParameterSet&);
    ~H2TestBeamAnalyzer();

    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
    virtual void beginJob() ;
    virtual void analyze(const edm::Event&, const edm::EventSetup&);
    void getData(const edm::Event&, const edm::EventSetup&);
    virtual void endJob() ;

    TFile *_file;
    TTree *_treeHBHE;
    TTree *_treeHF;
    TTree *_treeQIE11;
    TTree *_treeTriggers;
    TTree *_treeWC;
    TTree *_treeBC;
    TTree *_treeTiming;


    string _outFileName;
    int _verbosity;
    double gain_;
    TCalibLedInfo _calibInfo;
    TQIE8Info _hbheInfo;
    TQIE8Info _hfInfo;
    TQIE11Info _qie11Info;

    H2Triggers _triggers;
    H2BeamCounters _BCData;
    H2Timing _timing;

    vector<double> wcX[5];
    vector<double> wcY[5];

    TH1D *x[5];
    TH1D *y[5];
    TH1D *s1, *s2, *s3, *s4;

    virtual void beginRun(edm::Run const&, edm::EventSetup const&);
    virtual void endRun(edm::Run const&, edm::EventSetup const&);
    virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);
    virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);

    edm::EDGetTokenT<HBHEDigiCollection> tok_HBHEDigiCollection_;
    edm::EDGetTokenT<HFDigiCollection> tok_HFDigiCollection_;
    edm::EDGetTokenT<HODigiCollection> tok_HODigiCollection_;
    edm::EDGetTokenT<HcalDataFrameContainer<QIE11DataFrame> > tok_QIE11DigiCollection_;
    edm::EDGetTokenT<HcalTBTriggerData> tok_HcalTBTriggerData_;
    edm::EDGetTokenT<HcalTBEventPosition> tok_HcalTBEventPosition_;
    edm::EDGetTokenT<HcalTBBeamCounters> tok_HcalTBBeamCounters_;
    edm::EDGetTokenT<HcalTBParticleId> tok_HcalTBParticleId_;
    edm::EDGetTokenT<HcalTBTiming> tok_HcalTBTiming_;    
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
H2TestBeamAnalyzer::H2TestBeamAnalyzer(const edm::ParameterSet& iConfig) :
    _outFileName(iConfig.getUntrackedParameter<string>("OutFileName")),
    _verbosity(iConfig.getUntrackedParameter<int>("Verbosity")),
    gain_(iConfig.getUntrackedParameter<double>("Gain"))
{

    tok_HBHEDigiCollection_ = consumes<HBHEDigiCollection>(edm::InputTag("hcalDigis"));
    tok_HFDigiCollection_ = consumes<HFDigiCollection>(edm::InputTag("hcalDigis"));
    tok_HODigiCollection_ = consumes<HODigiCollection>(edm::InputTag("hcalDigis"));
    tok_QIE11DigiCollection_ = consumes<HcalDataFrameContainer<QIE11DataFrame> >(edm::InputTag("hcalDigis"));
    tok_HcalTBTriggerData_ = consumes<HcalTBTriggerData>(edm::InputTag("tbunpack"));
    tok_HcalTBEventPosition_ = consumes<HcalTBEventPosition>(edm::InputTag("tbunpack"));
    tok_HcalTBBeamCounters_ = consumes<HcalTBBeamCounters>(edm::InputTag("tbunpack"));
    tok_HcalTBParticleId_ = consumes<HcalTBParticleId>(edm::InputTag("tbunpack"));
    tok_HcalTBTiming_ = consumes<HcalTBTiming>(edm::InputTag("tbunpack"));

   //now do what ever initialization is needed

    _file = new TFile(_outFileName.c_str(), "recreate");
    _file->mkdir("HBHEData");
    _file->mkdir("HFData");
    _file->mkdir("QIE11Data");
    _file->mkdir("Triggers");
    _file->mkdir("WCData");
    _file->mkdir("Timing");
    _file->mkdir("BeamCounters");
    _file->mkdir("Histos");

/*
    _file->cd("DiodeInfo");
    _tree = new TTree("Events", "Events");
    _tree->Branch("numChs", &_calibInfo.numChs, "numChs/I");
    _tree->Branch("iphi", _calibInfo.iphi, "iphi[numChs]/I");
    _tree->Branch("ieta", _calibInfo.ieta, "ieta[numChs]/I");
    _tree->Branch("cBoxChannel", _calibInfo.cBoxChannel, "cBoxChannel[numChs]/I");
    _tree->Branch("cBoxString", &_calibInfo.cBoxString);
    _tree->Branch("nTS", _calibInfo.nTS, "nTS[numChs]/I");
    _tree->Branch("pulse", _calibInfo.pulse, "pulse[numChs][10]/D");
*/

    _file->cd("HBHEData");
    _treeHBHE = new TTree("Events", "Events");
    _treeHBHE->Branch("numChs", &_hbheInfo.numChs, "numChs/I");
    _treeHBHE->Branch("numTS", &_hbheInfo.numTS, "numTS/I");
    _treeHBHE->Branch("iphi", _hbheInfo.iphi, "iphi[numChs]/I");
    _treeHBHE->Branch("ieta", _hbheInfo.ieta, "ieta[numChs]/I");
    _treeHBHE->Branch("depth", _hbheInfo.depth, "depth[numChs]/I");
    _treeHBHE->Branch("pulse", _hbheInfo.pulse, "pulse[numChs][50]/D");
    _treeHBHE->Branch("ped", _hbheInfo.ped, "ped[numChs]/D");
    _treeHBHE->Branch("pulse_adc", _hbheInfo.pulse_adc, "pulse_adc[numChs][50]/D");
    _treeHBHE->Branch("ped_adc", _hbheInfo.ped_adc, "ped_adc[numChs]/D");
    _treeHBHE->Branch("valid", _hbheInfo.valid, "valid[numChs]/O");

    _file->cd("HFData");
    _treeHF = new TTree("Events", "Events");
    _treeHF->Branch("numChs", &_hfInfo.numChs, "numChs/I");
    _treeHF->Branch("numTS", &_hfInfo.numTS, "numTS/I");
    _treeHF->Branch("iphi", _hfInfo.iphi, "iphi[numChs]/I");
    _treeHF->Branch("ieta", _hfInfo.ieta, "ieta[numChs]/I");
    _treeHF->Branch("depth", _hfInfo.depth, "depth[numChs]/I");
    _treeHF->Branch("pulse", _hfInfo.pulse, "pulse[numChs][50]/D");
    _treeHF->Branch("pulse_adc", _hfInfo.pulse_adc, "pulse_adc[numChs][50]/D");
    _treeHF->Branch("ped", _hfInfo.ped, "ped[numChs]/D");
    _treeHF->Branch("ped_adc", _hfInfo.ped_adc, "ped_adc[numChs]/D");
    _treeHF->Branch("valid", _hfInfo.valid, "valid[numChs]/O");

    _file->cd("QIE11Data");
    _treeQIE11 = new TTree("Events", "Events");
    _treeQIE11->Branch("numChs", &_qie11Info.numChs, "numChs/I");
    _treeQIE11->Branch("numTS", &_qie11Info.numTS, "numTS/I");
    _treeQIE11->Branch("iphi", _qie11Info.iphi, "iphi[numChs]/I");
    _treeQIE11->Branch("ieta", _qie11Info.ieta, "ieta[numChs]/I");
    _treeQIE11->Branch("depth", _qie11Info.depth, "depth[numChs]/I");
    _treeQIE11->Branch("pulse", _qie11Info.pulse, "pulse[numChs][50]/D");
    _treeQIE11->Branch("ped", _qie11Info.ped, "ped[numChs]/D");
    _treeQIE11->Branch("pulse_adc", _qie11Info.pulse_adc, "pulse_adc[numChs][50]/D");
    _treeQIE11->Branch("ped_adc", _qie11Info.ped_adc, "ped_adc[numChs]/D");
    _treeQIE11->Branch("capid_error", _qie11Info.capid_error, "capid_error[numChs]/O");
    _treeQIE11->Branch("link_error", _qie11Info.link_error, "link_error[numChs]/O");
    _treeQIE11->Branch("soi", _qie11Info.soi, "soi[numChs][50]/O");

    _file->cd("Triggers");
    _treeTriggers = new TTree("Events", "Events");
    _treeTriggers->Branch("ped", &_triggers.ped, "ped/I");
    _treeTriggers->Branch("led", &_triggers.led, "led/I");
    _treeTriggers->Branch("laser", &_triggers.laser, "laser/I");
    _treeTriggers->Branch("beam", &_triggers.beam, "beam/I");
//  _treeTriggers->Branch("str", &_triggers.str, "char[200]")
    _treeTriggers->Branch("fakeTrg", &_triggers.fakeTrg, "fakeTrg/I");
    _treeTriggers->Branch("inSpillTrg", &_triggers.inSpillTrg, "inSpillTrg/I");

    _file->cd("WCData");
    _treeWC = new TTree("Events", "Events");
    _treeWC->Branch("xA", &(wcX[0]));
    _treeWC->Branch("yA", &(wcY[0]));
    _treeWC->Branch("xB", &(wcX[1]));
    _treeWC->Branch("yB", &(wcY[1]));
    _treeWC->Branch("xC", &(wcX[2]));
    _treeWC->Branch("yC", &(wcY[2]));
    _treeWC->Branch("xD", &(wcX[3]));
    _treeWC->Branch("yD", &(wcY[3]));
    _treeWC->Branch("xE", &(wcX[4]));
    _treeWC->Branch("yE", &(wcY[4]));

    _file->cd("Timing");
    _treeTiming = new TTree("Events", "Events");
    _treeTiming->Branch("s1Count", &_timing.s1Count, "s1Count/I");
    _treeTiming->Branch("s2Count", &_timing.s2Count, "s2Count/I");
    _treeTiming->Branch("s3Count", &_timing.s3Count, "s3Count/I");
    _treeTiming->Branch("s4Count", &_timing.s4Count, "s4Count/I");
    _treeTiming->Branch("triggerTime", &_timing.triggerTime, "triggerTime/D");
    _treeTiming->Branch("ttcL1Atime", &_timing.ttcL1Atime, "ttcL1Atime/D");

    _file->cd("Histos");
    s1 = new TH1D("s1", "s1", 10000, 0, 1000);
    s2 = new TH1D("s2", "s2", 10000, 0, 1000);
    s3 = new TH1D("s3", "s3", 10000, 0, 1000);
    s4 = new TH1D("s4", "s4", 10000, 0, 1000);
    x[0] = new TH1D("xA", "xA", 10000, -100, 100);
    x[1] = new TH1D("xB", "xB", 10000, -100, 100);
    x[2] = new TH1D("xC", "xC", 10000, -100, 100);
    x[3] = new TH1D("xD", "xD", 10000, -100, 100);
    x[4] = new TH1D("xE", "xE", 10000, -100, 100);
    y[0] = new TH1D("yA", "yA", 10000, -100, 100);
    y[1] = new TH1D("yB", "yB", 10000, -100, 100);
    y[2] = new TH1D("yC", "yC", 10000, -100, 100);
    y[3] = new TH1D("yD", "yD", 10000, -100, 100);
    y[4] = new TH1D("yE", "yE", 10000, -100, 100);

    _file->cd("BeamCounters");
    _treeBC = new TTree("Events", "Events");
    _treeBC->Branch("s1adc", &_BCData.s1adc, "s1adc/D");
    _treeBC->Branch("s2adc", &_BCData.s2adc, "s2adc/D");
    _treeBC->Branch("s3adc", &_BCData.s3adc, "s3adc/D");
    _treeBC->Branch("s4adc", &_BCData.s4adc, "s4adc/D");
}


H2TestBeamAnalyzer::~H2TestBeamAnalyzer()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

    _file->cd("Histos");
    s1->Write();
    s2->Write();
    s3->Write();
    s4->Write();
    x[0]->Write();
    x[1]->Write();
    x[2]->Write();
    x[3]->Write();
    x[4]->Write();
    y[0]->Write();
    y[1]->Write();
    y[2]->Write();
    y[3]->Write();
    y[4]->Write();

    _file->Write();
    _file->Close();
}

void H2TestBeamAnalyzer::getData(const edm::Event &iEvent, 
                const edm::EventSetup &iSetup)
{
    using namespace edm;

    //
    //  Extracting All the Collections containing useful Info
    //
    edm::Handle<HBHEDigiCollection> hbheDigiCollection;
    edm::Handle<HFDigiCollection> hfDigiCollection;
    edm::Handle<HODigiCollection> hoDigiCollection;
    edm::Handle<QIE11DigiCollection> qie11DigiCollection;
    edm::Handle<HcalTBTriggerData> trigData;
    edm::Handle<HcalTBEventPosition> eventPos;
    edm::Handle<HcalTBBeamCounters> beamCounters;
    edm::Handle<HcalTBParticleId> pId;
    edm::Handle<HcalTBTiming> timing;

    iEvent.getByToken(tok_HBHEDigiCollection_,hbheDigiCollection);
    iEvent.getByToken(tok_HFDigiCollection_,hfDigiCollection);
    iEvent.getByToken(tok_HODigiCollection_,hoDigiCollection);
    iEvent.getByToken(tok_QIE11DigiCollection_,qie11DigiCollection);
    iEvent.getByToken(tok_HcalTBTriggerData_,trigData);
    iEvent.getByToken(tok_HcalTBEventPosition_,eventPos);
    iEvent.getByToken(tok_HcalTBBeamCounters_,beamCounters);
    iEvent.getByToken(tok_HcalTBParticleId_,pId);
    iEvent.getByToken(tok_HcalTBTiming_,timing);
    
    //
    //  Extract Trigger Info
    //
    _triggers.ped       = (trigData->wasInSpillPedestalTrigger() || 
                           trigData->wasOutSpillPedestalTrigger() ||
                           trigData->wasSpillIgnorantPedestalTrigger());
    _triggers.led        = trigData->wasLEDTrigger();
    _triggers.laser      = trigData->wasLaserTrigger();
    _triggers.beam       = trigData->wasBeamTrigger();
    _triggers.fakeTrg    = trigData->wasFakeTrigger();
    _triggers.inSpillTrg = trigData->wasInSpill();
    _triggers.str        = trigData->runNumberSequenceId();
    
    //
    //  Extract Event Position
    //
    const char chambers[5] = {'A', 'B', 'C', 'D', 'E'};
    for (size_t i = 0; i < 5; ++i) {
      vector<double> xxx, yyy;
      eventPos->getChamberHits(chambers[i], xxx, yyy);
      wcX[i] = xxx;
      wcY[i] = yyy;
      
      for (size_t j = 0; j < xxx.size(); ++j) x[i]->Fill(xxx[j]);
      for (size_t j = 0; j < yyy.size(); ++j) y[i]->Fill(yyy[j]);
    }
    
    //
    //  Extract Beam Counters Info
    //
    _BCData.cer1adc = beamCounters->CK1adc();   
    _BCData.cer2adc = beamCounters->CK2adc();
    _BCData.cer3adc = beamCounters->CK3adc();
    _BCData.s1adc = beamCounters->S1adc();
    _BCData.s2adc = beamCounters->S2adc();
    _BCData.s3adc = beamCounters->S3adc();
    _BCData.s4adc = beamCounters->S4adc();

    s1->Fill(_BCData.s1adc);
    s2->Fill(_BCData.s2adc);
    s3->Fill(_BCData.s3adc);
    s4->Fill(_BCData.s4adc);

    //
    //  Extract Timing Info     
    //
    _timing.s1Count = timing->S1Count();
    _timing.s2Count = timing->S2Count();
    _timing.s3Count = timing->S3Count();
    _timing.s4Count = timing->S4Count();
    _timing.triggerTime = timing->triggerTime();
    _timing.ttcL1Atime = timing->ttcL1Atime();
    
    if (_verbosity>0)
    {
        cout << "### Before Loop: " << endl
            << "### HBHE Digis=" << hbheDigiCollection->size() << endl
            << "### HF Digis=" << hfDigiCollection->size() << endl
            << "### HO Digis=" << hoDigiCollection->size() << endl
            << "### QIE11 Digis=" << qie11DigiCollection->size() << endl;

        cout << "### Triggers: " << endl
            << "### PED Trigger: " << _triggers.ped << endl
            << "### Led Trigger: " << _triggers.led << endl
            << "### Laser Trigger: " << _triggers.laser << endl
            << "### Beam Trigger: " << _triggers.beam << endl
            << "### In Spill Trigger: " << _triggers.inSpillTrg << endl;
        /*
        cout << "### Wire Chamber A: NHits=" << _vWCData[0].x.size() << endl
            << "### Wire Chamber B: NHits=" << _vWCData[1].x.size() << endl
            << "### Wire Chamber C: NHits=" << _vWCData[2].x.size() << endl
            << "### Wire Chamber D: NHits=" << _vWCData[3].x.size() << endl
            << "### Wire Chamber E: NHits=" << _vWCData[4].x.size() << endl;
        cout << "HF Table Pos(X, Y, V) = " << tableX << "  " << tableY << "  " <<
            tableV << endl;
            */

        cout << "### Beam Counters:" << endl
            << "### Cerenkov1: " << _BCData.cer1adc << endl
            << "### Cerenkov2: " << _BCData.cer2adc << endl
            << "### Cerenkov3: " << _BCData.cer3adc << endl
            << "### s1adc=" << _BCData.s1adc << endl
            << "### s2adc=" << _BCData.s2adc << endl
            << "### s3adc=" << _BCData.s3adc << endl
            << "### s4adc=" << _BCData.s4adc << endl;

        cout << "### Beam Timing: " << endl
            << "### S1Count: " << _timing.s1Count << endl
            << "### S2Count: " << _timing.s2Count << endl
            << "### S3Count: " << _timing.s3Count << endl
            << "### S4Count: " << _timing.s4Count << endl;
    }
    
    int numChs = 0;
    
    for (HBHEDigiCollection::const_iterator digi=hbheDigiCollection->begin();
         digi!=hbheDigiCollection->end(); ++digi)
    {
        
        int iphi = digi->id().iphi();
        int ieta = digi->id().ieta();
        int depth = digi->id().depth();
        int nTS = digi->size();
        
        if (_verbosity>1)
        {
            int fiberChanId = digi->elecId().fiberChanId();
            int fiberIndex = digi->elecId().fiberIndex();
            int slbChannelIndex = digi->elecId().slbChannelIndex();
            int slbSiteNumber = digi->elecId().slbSiteNumber();
            string slbChannelCode = digi->elecId().slbChannelCode();
            int htrChanId = digi->elecId().htrChanId();
            int spigot = digi->elecId().spigot();
            int dccid = digi->elecId().dccid();
            int htrSlot = digi->elecId().htrSlot();
            int htrTopBottom = digi->elecId().htrTopBottom();
            int readoutVMECrateId = digi->elecId().readoutVMECrateId();
            int linearIndex = digi->elecId().linearIndex();
            
            cout << "### Digi->elecId:" << endl;
            cout << fiberChanId << "  " << fiberIndex << "  "
            << slbChannelIndex << "  " << slbSiteNumber << "  "
            << slbChannelCode << "  " << htrChanId << "  "
            << spigot << "  " << dccid << "  " << htrSlot << "  "
            << htrTopBottom << "  " << readoutVMECrateId << "  "
            << linearIndex << endl;
            cout << "### Digi->detId:" << endl;
            cout << iphi << "  " << ieta << "  " << depth << endl;
        }
        
        //
        //      Set the Branched arrays
        //
        _hbheInfo.iphi[numChs] = iphi;
        _hbheInfo.ieta[numChs] = ieta;
        _hbheInfo.depth[numChs] = depth;
        _hbheInfo.numTS = nTS;
        
        float ped_fc = 0;
        float ped_adc = 0;
        
        for (int iTS=0; iTS<nTS; iTS++)
        {
            const unsigned char adc = digi->sample(iTS).adc() & 0xff;
            _hbheInfo.pulse[numChs][iTS] = adc2fC[adc];
            _hbheInfo.pulse_adc[numChs][iTS] = adc;
            if (iTS < 3)
            {
                ped_fc += adc2fC[adc];
                ped_adc += adc;
            }
        }
        
        _hbheInfo.ped[numChs] = ped_fc/3.;
        _hbheInfo.ped_adc[numChs] = ped_adc/3.;
        
        _hbheInfo.valid[numChs] = digi->validate();
        
        if (_verbosity>1)
        {
            cout << "### Digi->Data:" << endl;
            for (int iTS=0; iTS<nTS; iTS++)
                cout << _hbheInfo.pulse[numChs][iTS] << "  ";
            cout << endl;
        }
        
        numChs++;
    }
    _hbheInfo.numChs = numChs;


    numChs = 0;
    for (HFDigiCollection::const_iterator digi=hfDigiCollection->begin();
                        digi!=hfDigiCollection->end(); ++digi)
    {

        int iphi = digi->id().iphi();
        int ieta = digi->id().ieta();
        int depth = digi->id().depth();
        int nTS = digi->size();

        if (_verbosity>1)
        {
            int fiberChanId = digi->elecId().fiberChanId();
            int fiberIndex = digi->elecId().fiberIndex();
            int slbChannelIndex = digi->elecId().slbChannelIndex();
            int slbSiteNumber = digi->elecId().slbSiteNumber();
            string slbChannelCode = digi->elecId().slbChannelCode();
            int htrChanId = digi->elecId().htrChanId();
            int spigot = digi->elecId().spigot();
            int dccid = digi->elecId().dccid();
            int htrSlot = digi->elecId().htrSlot();
            int htrTopBottom = digi->elecId().htrTopBottom();
            int readoutVMECrateId = digi->elecId().readoutVMECrateId();
            int linearIndex = digi->elecId().linearIndex();

            cout << "### Digi->elecId:" << endl;
            cout << fiberChanId << "  " << fiberIndex << "  "
                << slbChannelIndex << "  " << slbSiteNumber << "  "
                << slbChannelCode << "  " << htrChanId << "  "
                << spigot << "  " << dccid << "  " << htrSlot << "  "
                << htrTopBottom << "  " << readoutVMECrateId << "  "
                << linearIndex << endl;
            cout << "### Digi->detId:" << endl;
            cout << iphi << "  " << ieta << "  " << depth << endl;
        }

        //
        //      Set the Branched arrays
        //
        _hfInfo.iphi[numChs] = iphi;
        _hfInfo.ieta[numChs] = ieta;
        _hfInfo.depth[numChs] = depth;
        _hfInfo.numTS = nTS;
        
        float ped_fc = 0;
        float ped_adc = 0;

        for (int iTS=0; iTS<nTS; iTS++)
        {
            const unsigned char adc = digi->sample(iTS).adc() & 0xff;
            _hfInfo.pulse[numChs][iTS] = adc2fC[adc];
            _hfInfo.pulse_adc[numChs][iTS] = adc;
            if (iTS < 3)
            {
                ped_fc += adc2fC[adc];
                ped_adc += adc;
            }
        }
        
        _hfInfo.ped[numChs] = ped_fc/3.;
        _hfInfo.ped_adc[numChs] = ped_adc/3.;
        
        _hfInfo.valid[numChs] = digi->validate();

        if (_verbosity>1)
        {
            cout << "### Digi->Data:" << endl;
            for (int iTS=0; iTS<nTS; iTS++)
                cout << _hfInfo.pulse[numChs][iTS] << "  ";
            cout << endl;
        }

        numChs++;
    }
    _hfInfo.numChs = numChs;


    // --------------------------
    // --   QIE11 Information  --
    // --------------------------
    
    // This does not work:
    // for (QIE11DigiCollection::const_iterator digi=qie11DigiCollection->begin();
    //   digi!=qie11DigiCollection->end(); ++digi)
    //   cout << digi->samples() << endl;
    // But this apparently does..
    
    
    if (_verbosity>0) std::cout << "Trying to access the qie collection" << std::endl;
    
    Converter Convertadc2fC(gain_);

    const QIE11DigiCollection& qie11dc=*(qie11DigiCollection);

    for (int j=0; j < qie11dc.size(); j++){
        
        if (_verbosity>0){
            std::cout << "Printing raw dataframe" << std::endl;
            std::cout << qie11dc[j] << std::endl;
            
            std::cout << "Printing content of samples() method" << std::endl;
            std::cout << qie11dc[j].samples() << std::endl;
        }
        
        // Extract info on detector location
        DetId detid = qie11dc[j].detid();
        HcalDetId hcaldetid = HcalDetId(detid);
        int ieta = hcaldetid.ieta();
        int iphi = hcaldetid.iphi();
        int depth = hcaldetid.depth();
        
        if (_verbosity>0){
            std::cout << "Where am I?\n detid: " << detid.rawId() << std::endl;
            std::cout << " ieta: " << ieta << "\n"
            << " iphi: " << iphi << "\n"
            << " depth: " << depth << std::endl;
        }
        
        // loop over the samples in the digi
        int nTS = qie11dc[j].samples();

        float ped_adc = 0;
        float ped_fc = 0;

        for(int i=0; i<nTS; ++i)
        {
            int adc = qie11dc[j][i].adc();
            int tdc = qie11dc[j][i].tdc();
            int capid = qie11dc[j][i].capid();
            int soi = qie11dc[j][i].soi();
            
            // store pulse information
            float charge = Convertadc2fC.linearize(adc);
            _qie11Info.pulse[j][i] = charge;
            _qie11Info.pulse_adc[j][i] = adc;
            _qie11Info.soi[j][i] = soi;

            if (_verbosity>0)
                std::cout << "Sample " << i << ": ADC=" << adc << " Charge=" << charge << "fC" << " TDC=" << tdc << " Capid=" << capid
                          << " SOI=" << soi << std::endl;

            // compute ped from first 3 time samples
            if (i<3){
                ped_adc += adc;
                ped_fc += charge;
            }
            
        }
        ped_adc = ped_adc/3.;
        ped_fc = ped_fc/3.; 

        if (_verbosity>0)
            std::cout << "The pedestal for this channel is " << ped_adc << "ADC counts and " << ped_fc << " fC" << std::endl;
  
        // -------------------------------------
        // --    Set the Branched arrays      --
        // -------------------------------------
        _qie11Info.iphi[j] = iphi;
        _qie11Info.ieta[j] = ieta;
        _qie11Info.depth[j] = depth;
        _qie11Info.ped[j] = ped_fc;
        _qie11Info.ped_adc[j] = ped_adc;
        _qie11Info.capid_error[j] = qie11dc[j].capidError();
        _qie11Info.link_error[j] = qie11dc[j].linkError();
    }

    _qie11Info.numChs = qie11dc.size();
    _qie11Info.numTS = qie11dc.samples();

    _treeHBHE->Fill();
    _treeHF->Fill();
    _treeQIE11->Fill();
    _treeTriggers->Fill();
    _treeWC->Fill();
    _treeBC->Fill();
    _treeTiming->Fill();

    return;
}


//
// member functions
//

// ------------ method called for each event  ------------
void H2TestBeamAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;
   getData(iEvent, iSetup);
}


// ------------ method called once each job just before starting event loop  ------------
// ------------ method called once each job just after ending the event loop  ------------
// ------------ method called when starting to processes a run  ------------
// ------------ method called when ending the processing of a run  ------------
// ------------ method called when starting to processes a luminosity block  ------------
// ------------ method called when ending the processing of a luminosity block  ------------
void H2TestBeamAnalyzer::beginJob() {}
void H2TestBeamAnalyzer::endJob() {}
void H2TestBeamAnalyzer::beginRun(edm::Run const&, edm::EventSetup const&) {}
void H2TestBeamAnalyzer::endRun(edm::Run const&, edm::EventSetup const&) {}
void H2TestBeamAnalyzer::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) {}
void H2TestBeamAnalyzer::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) {}


// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void H2TestBeamAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions)
{
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(H2TestBeamAnalyzer);
