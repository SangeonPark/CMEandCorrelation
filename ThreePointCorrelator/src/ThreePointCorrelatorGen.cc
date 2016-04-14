// -*- C++ -*-
//
// Package:    ThreePointCorrelatorGen
// Class:      ThreePointCorrelatorGen
// 
/**\class ThreePointCorrelatorGen ThreePointCorrelatorGen.cc CMEandCorrelation/ThreePointCorrelatorGen/src/ThreePointCorrelatorGen.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Zhoudunming Tu,42 1-015,,
//         Created:  Mon Feb 15 10:38:19 CET 2016
// $Id$
//
//

#include "CMEandCorrelation/ThreePointCorrelator/interface/ThreePointCorrelatorBase.h"

ThreePointCorrelatorGen::ThreePointCorrelatorGen(const edm::ParameterSet& iConfig)

{
   //now do what ever initialization is needed
  trackSrc_ = iConfig.getParameter<edm::InputTag>("trackSrc");
  vertexSrc_ = iConfig.getParameter<std::string>("vertexSrc");
  towerSrc_ = iConfig.getParameter<edm::InputTag>("towerSrc");
  genParticleSrc_ = iConfig.getParameter<edm::InputTag>("genParticleSrc");

  Nmin_ = iConfig.getUntrackedParameter<int>("Nmin");
  Nmax_ = iConfig.getUntrackedParameter<int>("Nmax");
  
  useCentrality_ = iConfig.getUntrackedParameter<bool>("useCentrality");
  useBothSide_ = iConfig.getUntrackedParameter<bool>("useBothSide");  
  reverseBeam_ = iConfig.getUntrackedParameter<bool>("reverseBeam");
  messAcceptance_ = iConfig.getUntrackedParameter<bool>("messAcceptance");

  etaLowHF_ = iConfig.getUntrackedParameter<double>("etaLowHF");
  etaHighHF_ = iConfig.getUntrackedParameter<double>("etaHighHF");
  vzLow_ = iConfig.getUntrackedParameter<double>("vzLow");
  vzHigh_ = iConfig.getUntrackedParameter<double>("vzHigh");
  ptLow_ = iConfig.getUntrackedParameter<double>("ptLow");
  ptHigh_ = iConfig.getUntrackedParameter<double>("ptHigh");
  holeLeft_ = iConfig.getUntrackedParameter<double>("holeLeft");
  holeRight_ = iConfig.getUntrackedParameter<double>("holeRight");

  offlineptErr_ = iConfig.getUntrackedParameter<double>("offlineptErr", 0.0);
  offlineDCA_ = iConfig.getUntrackedParameter<double>("offlineDCA", 0.0);

  holesize_ = iConfig.getUntrackedParameter<double>("holesize");

  etaBins_ = iConfig.getUntrackedParameter<std::vector<double>>("etaBins");
  dEtaBins_ = iConfig.getUntrackedParameter<std::vector<double>>("dEtaBins");

}


ThreePointCorrelatorGen::~ThreePointCorrelatorGen()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}

// ------------ method called for each event  ------------
void
ThreePointCorrelatorGen::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;
  using namespace std;

  edm::Handle<reco::VertexCollection> vertices;
  iEvent.getByLabel(vertexSrc_,vertices);
  double bestvz=-999.9, bestvx=-999.9, bestvy=-999.9;
  double bestvzError=-999.9, bestvxError=-999.9, bestvyError=-999.9;
  const reco::Vertex & vtx = (*vertices)[0];
  bestvz = vtx.z(); bestvx = vtx.x(); bestvy = vtx.y();
  bestvzError = vtx.zError(); bestvxError = vtx.xError(); bestvyError = vtx.yError();
  
  //first selection; vertices
  if(bestvz < vzLow_ || bestvz > vzHigh_ ) return;
  
  Handle<CaloTowerCollection> towers;
  iEvent.getByLabel(towerSrc_, towers);

  Handle<reco::TrackCollection> tracks;
  iEvent.getByLabel(trackSrc_, tracks);
  
  if( useCentrality_ ){

    CentralityProvider * centProvider = 0;
    if (!centProvider) centProvider = new CentralityProvider(iSetup);
    centProvider->newEvent(iEvent,iSetup);
    int hiBin = centProvider->getBin();
    cbinHist->Fill( hiBin );
    if( hiBin < Nmin_ || hiBin >= Nmax_ ) return;

  }

  const int NetaBins = etaBins_.size() - 1 ;
  const int NdEtaBins = dEtaBins_.size() - 1;
  double dEtaBinsArray[48];

  for(unsigned i = 0; i < dEtaBins_.size(); i++){

    dEtaBinsArray[i] = dEtaBins_[i]-0.0001;
  }

  int nTracks = 0;
  for(unsigned it = 0; it < tracks->size(); it++){

     const reco::Track & trk = (*tracks)[it];

     math::XYZPoint bestvtx(bestvx,bestvy,bestvz);
        
        double dzvtx = trk.dz(bestvtx);
        double dxyvtx = trk.dxy(bestvtx);
        double dzerror = sqrt(trk.dzError()*trk.dzError()+bestvzError*bestvzError);
        double dxyerror = sqrt(trk.d0Error()*trk.d0Error()+bestvxError*bestvyError);

        if(!trk.quality(reco::TrackBase::highPurity)) continue;
        if(fabs(trk.ptError())/trk.pt() > offlineptErr_ ) continue;
        if(fabs(dzvtx/dzerror) > offlineDCA_) continue;
        if(fabs(dxyvtx/dxyerror) > offlineDCA_) continue;
        if(fabs(trk.eta()) < 2.4 && trk.pt() > 0.4 ){nTracks++;}// NtrkOffline        
           
  } 

  if( !useCentrality_ ) if( nTracks < Nmin_ || nTracks >= Nmax_ ) return;
  
  Ntrk->Fill(nTracks);

//loop over calo towers (HF)
// initialize Qcos and Qsin

  double QcosTRK = 0.;
  double QsinTRK = 0.;
  double QcountsTrk = 0;

  double Q1[NetaBins][2][2];
  double Q1_count[NetaBins][2];

  double Q2[NetaBins][2][2];
  double Q2_count[NetaBins][2];

  for(int i = 0; i < NetaBins; i++){
    for(int j = 0; j < 2; j++){
      Q1_count[i][j] = 0.0;
      Q2_count[i][j] = 0.0;
      for(int k = 0; k < 2; k++){
        Q1[i][j][k] = 0.0;
        Q2[i][j][k] = 0.0;
      }
    }
  }

  double Q3[2][2];
  double ETT[2];

  for(int i = 0; i < 2; i++){
    ETT[i] = 0.;
    for(int j = 0; j < 2; j++){
      Q3[i][j] = 0.;
    }
  }

  int HFside = 2;
  if( useBothSide_ ) HFside = 1;

  edm::Handle<reco::GenParticleCollection> genParticleCollection;
  iEvent.getByLabel(genParticleSrc_, genParticleCollection);

  for(unsigned it=0; it<genParticleCollection->size(); ++it) {

    const reco::GenParticle & genCand = (*genParticleCollection)[it];
    int status = genCand.status();
    double genpt = genCand.pt();
    double geneta = genCand.eta();
    double genphi = genCand.phi();
    int gencharge = genCand.charge();

    if( status != 1  || gencharge == 0 ) continue;

    if( fabs(geneta) < 2.4 ){

      QcosTRK += cos( 2*genphi );
      QsinTRK += sin( 2*genphi );
      QcountsTrk++ ;

      for(int eta = 0; eta < NetaBins; eta++){
          if( geneta > etaBins_[eta] && geneta < etaBins_[eta+1] ){

            if( gencharge == 1 ){
              
              Q1[eta][0][0] += cos( genphi );
              Q1[eta][0][1] += sin( genphi );
              Q1_count[eta][0]++;

              Q2[eta][0][0] += cos( 2*genphi );
              Q2[eta][0][1] += sin( 2*genphi );
              Q2_count[eta][0]++;

            }
            else if( gencharge == -1 ){

              Q1[eta][1][0] += cos( genphi );
              Q1[eta][1][1] += sin( genphi );
              Q1_count[eta][1]++;

              Q2[eta][1][0] += cos( 2*genphi );
              Q2[eta][1][1] += sin( 2*genphi );
              Q2_count[eta][1]++;


            }
            else{cout << "break!" << endl; return;}
          }
      }
    }
    if( geneta < etaHighHF_ && geneta > etaLowHF_ ){
        
          Q3[0][0] += cos( -2*genphi );
          Q3[0][1] += sin( -2*genphi );
          ETT[0]++;
      }
      else if( geneta < -etaLowHF_ && geneta > -etaHighHF_ ){

          Q3[1][0] += cos( -2*genphi );
          Q3[1][1] += sin( -2*genphi );
          ETT[1]++;

      }
      else{continue;}
  }

  for(int ieta = 0; ieta < NetaBins; ieta++){
    for(int jeta = 0; jeta < NetaBins; jeta++){
    
      double deltaEta = fabs(etaBins_[ieta] - etaBins_[jeta]);

      for(int deta = 0; deta < NdEtaBins; deta++){
        if( deltaEta > dEtaBinsArray[deta] && deltaEta < dEtaBinsArray[deta+1] ){
          if( deta == 0 ){
            for(int HF = 0; HF < HFside; HF++){
              for(int sign = 0; sign < 2; sign++){

                if( Q1_count[ieta][sign] == 0.0 || ETT[HF] == 0.0 ) continue;

                double Q_real = get3RealOverlap(Q1[ieta][sign][0], Q2[ieta][sign][0], Q3[HF][0], Q1[ieta][sign][1], Q2[ieta][sign][1], Q3[HF][1], Q1_count[ieta][sign], ETT[HF] );
                QvsdEta[deta][sign][HF]->Fill( Q_real, Q1_count[ieta][sign]*(Q1_count[ieta][sign]-1)*ETT[HF] );
                
              }

              if( Q1_count[ieta][0] == 0.0 || Q1_count[ieta][1] == 0.0 || ETT[HF] == 0.0 ) continue;

              double Q_real = get3Real(Q1[ieta][0][0]/Q1_count[ieta][0],Q1[jeta][1][0]/Q1_count[jeta][1],Q3[HF][0]/ETT[HF], Q1[ieta][0][1]/Q1_count[ieta][0], Q1[jeta][1][1]/Q1_count[jeta][1], Q3[HF][1]/ETT[HF]);
              QvsdEta[deta][2][HF]->Fill( Q_real, Q1_count[ieta][0]*Q1_count[jeta][1]*ETT[HF] );  

            }
          }
          else{
            for(int HF = 0; HF < HFside; HF++){
              for(int sign = 0; sign < 2; sign++ ){
              
                if( Q1_count[ieta][sign] == 0.0 || Q1_count[jeta][sign] == 0.0 || ETT[HF] == 0.0 ) continue; 

                double Q_real = get3Real(Q1[ieta][sign][0]/Q1_count[ieta][sign],Q1[jeta][sign][0]/Q1_count[jeta][sign],Q3[HF][0]/ETT[HF], Q1[ieta][sign][1]/Q1_count[ieta][sign], Q1[jeta][sign][1]/Q1_count[jeta][sign], Q3[HF][1]/ETT[HF]);
                QvsdEta[deta][sign][HF]->Fill( Q_real, Q1_count[ieta][sign]*Q1_count[jeta][sign]*ETT[HF] );  

              }

              if( Q1_count[ieta][0] == 0.0 || Q1_count[jeta][1] == 0.0 || ETT[HF] == 0.0 ) continue;

                double Q_real = get3Real(Q1[ieta][0][0]/Q1_count[ieta][0],Q1[jeta][1][0]/Q1_count[jeta][1],Q3[HF][0]/ETT[HF], Q1[ieta][0][1]/Q1_count[ieta][0], Q1[jeta][1][1]/Q1_count[jeta][1], Q3[HF][1]/ETT[HF]);
                QvsdEta[deta][2][HF]->Fill( Q_real, Q1_count[ieta][0]*Q1_count[jeta][1]*ETT[HF] );  

            }
          }
        }
      }
    }
  }

/*
calculate v2 using 3 sub-events method:
 */

  QcosTRK = QcosTRK/QcountsTrk;
  QsinTRK = QsinTRK/QcountsTrk;

  double QaQc = get2Real(Q3[1][0]/ETT[1], QcosTRK/QcountsTrk, Q3[1][1]/ETT[1], QsinTRK/QcountsTrk );
  double QaQb = get2Real(Q3[1][0]/ETT[1], Q3[0][0]/ETT[0], Q3[1][1]/ETT[1], -Q3[0][1]/ETT[0]);//an extra minus sign 
  double QcQb = get2Real(QcosTRK/QcountsTrk, Q3[0][0]/ETT[0], QsinTRK/QcountsTrk, Q3[0][1]/ETT[0]);

  c2_ac->Fill( QaQc, ETT[1]*QcountsTrk );
  c2_cb->Fill( QcQb, ETT[1]*ETT[0]  );
  c2_ab->Fill( QaQb, ETT[0]*QcountsTrk );

}
// ------------ method called once each job just before starting event loop  ------------
void 
ThreePointCorrelatorGen::beginJob()
{

  edm::Service<TFileService> fs;
    
  TH1D::SetDefaultSumw2();

  edm::FileInPath fip1("CMEandCorrelation/ThreePointCorrelator/data/TrackCorrections_HIJING_538_OFFICIAL_Mar24.root");
  TFile f1(fip1.fullPath().c_str(),"READ");
  effTable = (TH2D*)f1.Get("rTotalEff3D");

  Ntrk = fs->make<TH1D>("Ntrk",";Ntrk",5000,0,5000);
  cbinHist = fs->make<TH1D>("cbinHist",";cbin",200,0,200);
  trkPhi = fs->make<TH1D>("trkPhi", ";#phi", 700, -3.5, 3.5);
  hfPhi = fs->make<TH1D>("hfPhi", ";#phi", 700, -3.5, 3.5);

  const int NdEtaBins = dEtaBins_.size() - 1;
  int HFside = 2;
  if( useBothSide_ ) HFside = 1;
//HF:
  c2_ab = fs->make<TH1D>("c2_ab",";c2_ab", 20000,-1,1);
  c2_ac = fs->make<TH1D>("c2_ac",";c2_ac", 20000,-1,1);
  c2_cb = fs->make<TH1D>("c2_cb",";c2_cb", 20000,-1,1);

  for(int deta = 0; deta < NdEtaBins; deta++){
    for(int sign = 0; sign < 3; sign++){
      for(int HF = 0; HF < HFside; HF++){       
        QvsdEta[deta][sign][HF] = fs->make<TH1D>(Form("QvsdEta_%d_%d_%d",deta,sign,HF), "", 20000,-1.0-0.00005, 1.0-0.00005);
      }
    }
  }





}
double 
ThreePointCorrelatorGen::get3Real(double R1, double R2, double R3, double I1, double I2, double I3) 
{

  double t1 = R1*R2*R3;
  double t2 = R1*I2*I3;
  double t3 = R2*I1*I3;
  double t4 = I1*I2*R3;

  return t1-t2-t3-t4;
}

double ThreePointCorrelatorGen::get3RealOverlap(double R1, double R2, double R3, double I1, double I2, double I3, double N1, double N3){

      double t1 = (R1*R1 - I1*I1 - R2)*R3;
      double t2 = (2*R1*I1-I2)*I3;
      double N = N1*(N1-1)*N3;

      return (t1-t2)/N;
}
double ThreePointCorrelatorGen::get3Imag(double R1, double R2, double R3, double I1, double I2, double I3){

  double t1 = R1*R2*I3;
  double t2 = R1*R3*I2;
  double t3 = R2*R3*I1;
  double t4 = I1*I2*I3;

  return t1+t2+t3-t4;

}
double ThreePointCorrelatorGen::get3ImagOverlap(double R1, double R2, double R3, double I1, double I2, double I3, double N1, double N3){

      double t1 = (R1*R1 - I1*I1 - R2)*I3;
      double t2 = (2*R1*I1-I2)*R3;
      double N = N1*(N1-1)*N3;

      return (t1+t2)/N;
}

double ThreePointCorrelatorGen::get2Real( double R1, double R2, double I1, double I2){

      double real = R1*R2 - I1*I2;
      return real;

}
double ThreePointCorrelatorGen::get2RealOverlap( double R1, double R2, double I1, double I2){

      double real = R1*R1 - I1*I1 - R2;
      return real;

}
double ThreePointCorrelatorGen::get2Imag( double R1, double R2, double I1, double I2){

      double imag = R1*I2 + R2*I1;
      return imag;
}
double ThreePointCorrelatorGen::get2ImagOverlap( double R1, double R2, double I1, double I2){

      double imag = (2*R1*I1-I2);
      return imag;
} 
// ------------ method called once each job just after ending the event loop  ------------
void 
ThreePointCorrelatorGen::endJob() 
{
}

// ------------ method called when starting to processes a run  ------------
void 
ThreePointCorrelatorGen::beginRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a run  ------------
void 
ThreePointCorrelatorGen::endRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when starting to processes a luminosity block  ------------
void 
ThreePointCorrelatorGen::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a luminosity block  ------------
void 
ThreePointCorrelatorGen::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
ThreePointCorrelatorGen::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(ThreePointCorrelatorGen);