#include <string>
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/ValueMap.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/PatCandidates/interface/libminifloat.h"

namespace pat {
    class PATVertexSlimmer : public edm::EDProducer {
        public:
            explicit PATVertexSlimmer(const edm::ParameterSet&);
            ~PATVertexSlimmer();

            virtual void produce(edm::Event&, const edm::EventSetup&);
        private:
            edm::EDGetTokenT<std::vector<reco::Vertex> > src_;
            edm::EDGetTokenT<edm::ValueMap<float> > score_;
            bool rekeyScores_;
    };
}

pat::PATVertexSlimmer::PATVertexSlimmer(const edm::ParameterSet& iConfig) :
    src_(consumes<std::vector<reco::Vertex> >(iConfig.getParameter<edm::InputTag>("src"))),
    score_(mayConsume<edm::ValueMap<float>>(iConfig.existsAs<edm::InputTag>("score")?iConfig.getParameter<edm::InputTag>("score"):edm::InputTag())),
    rekeyScores_(iConfig.existsAs<edm::InputTag>("score"))
{
  produces<std::vector<reco::Vertex> >();
  if(rekeyScores_) produces<edm::ValueMap<float> >();
}

pat::PATVertexSlimmer::~PATVertexSlimmer() {}

void pat::PATVertexSlimmer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
    edm::Handle<std::vector<reco::Vertex> > vertices;
    iEvent.getByToken(src_, vertices);
    std::auto_ptr<std::vector<reco::Vertex> > outPtr(new std::vector<reco::Vertex>());

    outPtr->reserve(vertices->size());
    for (unsigned int i = 0, n = vertices->size(); i < n; ++i) {
        const reco::Vertex &v = (*vertices)[i];
        auto co = v.covariance();
        if(i>0) {
          for(size_t j=0;j<3;j++){
            for(size_t k=j;k<3;k++){
              co(j,k) = MiniFloatConverter::reduceMantissaToNbits<10>( co(j,k) );
            }
          }
        }
        outPtr->push_back(reco::Vertex(v.position(), co, v.chi2(), v.ndof(), 0));
    }

    auto oh = iEvent.put(outPtr);
    if(rekeyScores_) {
      edm::Handle<edm::ValueMap<float> > scores;
      iEvent.getByToken(score_, scores);
      std::auto_ptr<edm::ValueMap<float> >  vertexScoreOutput( new edm::ValueMap<float> );
      edm::ValueMap<float>::const_iterator idIt=scores->begin();
      for(;idIt!=scores->end();idIt++) {
          if(idIt.id() ==  vertices.id()) break;
      }
      // std::find_if(scores->begin(), scores->end(), [vertices] (const edm::ValueMap<float>::const_iterator& s) { return s.id() == vertices.id(); } );
      edm::ValueMap<float>::Filler vertexScoreFiller(*vertexScoreOutput);
      vertexScoreFiller.insert(oh,idIt.begin(),idIt.end());
      vertexScoreFiller.fill();
      iEvent.put( vertexScoreOutput );
    }
}

using pat::PATVertexSlimmer;
#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATVertexSlimmer);
