import FWCore.ParameterSet.Config as cms

process = cms.Process("Demo")

#process.Timing = cms.Service("Timing")

import HLTrigger.HLTfilters.hltHighLevel_cfi
process.hltHM = HLTrigger.HLTfilters.hltHighLevel_cfi.hltHighLevel.clone()
process.hltHM.HLTPaths = ['HLT_PAPixelTracks_Multiplicity100_v*',
                          'HLT_PAPixelTracks_Multiplicity130_v*',
                          'HLT_PAPixelTracks_Multiplicity160_v*'
                          #'HLT_PAPixelTracks_Multiplicity190_v*'
                          #'HLT_PAPixelTracks_Multiplicity220_v*'
]

process.hltHM.andOr = cms.bool(True)
process.hltHM.throw = cms.bool(False)

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = cms.untracked.int32(1000)
process.options   = cms.untracked.PSet( wantSummary =
cms.untracked.bool(True) )
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

process.load("Configuration.StandardSequences.Digi_cff")
process.load('Configuration.StandardSequences.GeometryDB_cff')
process.load("Configuration.StandardSequences.RawToDigi_cff")
process.load("Configuration.EventContent.EventContent_cff")
process.load("Configuration.StandardSequences.Reconstruction_cff")
process.load("Configuration.StandardSequences.MagneticField_38T_cff")
process.load("SimGeneral.MixingModule.mixNoPU_cfi")

process.source = cms.Source("PoolSource",
    # replace 'myfile.root' with the source file you want to use
    fileNames = cms.untracked.vstring(
'root://xrootd-cms.infn.it//store/user/davidlw/PAHighPt/PA2013_FlowCorr_PromptReco_TrkHM_Gplus_ReTracking_v18/28b2b9cce04ec3f20baeb96fbd2295a8/pPb_HM_1000_1_BPd.root'    
#'root://xrootd-cms.infn.it//store/user/davidlw/HIMinBiasUPC/PR2011_MBPPRereco_TRKANASKIM_v6/ccf03100d177f42de0f9cdc7627799d3/pPb_HM_1000_1_lq0.root'
)
)

process.ana = cms.EDAnalyzer('ThreePointCorrelatorEtaGap',
						  vertexSrc = cms.string('offlinePrimaryVertices'),
                          			  trackSrc = cms.InputTag('generalTracks'),
						  towerSrc = cms.InputTag('towerMaker'),
						  offlineDCA = cms.untracked.double(3.0),
						  offlineptErr = cms.untracked.double(0.1),
					          useCentrality = cms.untracked.bool(False),
						  useBothSide = cms.untracked.bool(False),
						  Nmin = cms.untracked.int32(185),
						  Nmax = cms.untracked.int32(220),
						  etaLowHF = cms.untracked.double(4.4),
						  etaHighHF = cms.untracked.double(5.0),
						  etaBins = cms.untracked.vdouble(-2.4,-2.3,-2.2,-2.1,-2,-1.9,-1.8,-1.7,-1.6,-1.5,-1.4,
										  -1.3,-1.2,-1.1,-1,-0.9,-0.8,-0.7,-0.6,-0.5,-0.4,-0.3,
										  -0.2,-0.1,0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1,1.1,
										  1.2,1.3,1.4,1.5,1.6,1.7,1.8,1.9,2,2.1,2.2,2.3,2.4),
					          dEtaBins = cms.untracked.vdouble(0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0,
									   	   1.1,1.2,1.3,1.4,1.5,1.6,1.7,1.8,1.9,2.0,2.2,
										   2.4,2.6,2.8,3.0,3.4,3.8,4.2,4.8)
						
)



process.TFileService = cms.Service("TFileService",fileName = cms.string("test.root"))
process.p = cms.Path(   process.hltHM*
			process.ana
                                    )
