/*
 * \file EBPnDiodeClient.cc
 * 
 * $Date: 2005/12/01 09:37:12 $
 * $Revision: 1.9 $
 * \author G. Della Ricca
 * \author F. Cossutti
 *
*/

#include <DQM/EcalBarrelMonitorClient/interface/EBPnDiodeClient.h>

EBPnDiodeClient::EBPnDiodeClient(const edm::ParameterSet& ps, MonitorUserInterface* mui){

  mui_ = mui;

  for ( int i = 0; i < 36; i++ ) {

    h01_[i] = 0;
    h02_[i] = 0;
    h03_[i] = 0;
    h04_[i] = 0;

  }

}

EBPnDiodeClient::~EBPnDiodeClient(){

  for ( int i = 0; i < 36; i++ ) {

    if ( h01_[i] ) delete h01_[i];
    if ( h02_[i] ) delete h02_[i];
    if ( h03_[i] ) delete h03_[i];
    if ( h04_[i] ) delete h04_[i];

  }

}

void EBPnDiodeClient::beginJob(const edm::EventSetup& c){

  cout << "EBPnDiodeClient: beginJob" << endl;

  ievt_ = 0;

  this->subscribe();

}

void EBPnDiodeClient::beginRun(const edm::EventSetup& c){

  cout << "EBPnDiodeClient: beginRun" << endl;

  jevt_ = 0;

  for ( int ism = 1; ism <= 36; ism++ ) {

    if ( h01_[ism-1] ) delete h01_[ism-1];
    if ( h02_[ism-1] ) delete h02_[ism-1];
    if ( h03_[ism-1] ) delete h03_[ism-1];
    if ( h04_[ism-1] ) delete h04_[ism-1];
    h01_[ism-1] = 0;
    h02_[ism-1] = 0;
    h03_[ism-1] = 0;
    h04_[ism-1] = 0;

  }

}

void EBPnDiodeClient::endJob(void) {

  cout << "EBPnDiodeClient: endJob, ievt = " << ievt_ << endl;

  this->unsubscribe();

}

void EBPnDiodeClient::endRun(EcalCondDBInterface* econn, RunIOV* runiov, RunTag* runtag) {

  cout << "EBPnDiodeClient: endRun, jevt = " << jevt_ << endl;

  if ( jevt_ == 0 ) return;

  EcalLogicID ecid;
  MonPNDat p;
  map<EcalLogicID, MonPNDat> dataset;

  cout << "Writing MonPnDatObjects to database ..." << endl;

  const float n_min_tot = 1000.;
  const float n_min_bin = 50.;

  for ( int ism = 1; ism <= 36; ism++ ) {

    float num01, num02, num03, num04;
    float mean01, mean02, mean03, mean04;
    float rms01, rms02, rms03, rms04;

    for ( int i = 1; i <= 10; i++ ) {

      num01  = num02  = num03  = num04  = -1.;
      mean01 = mean02 = mean03 = mean04 = -1.;
      rms01  = rms02  = rms03  = rms04  = -1.;

      bool update_channel = false;

      if ( h01_[ism-1] && h01_[ism-1]->GetEntries() >= n_min_tot ) {
        num01 = h01_[ism-1]->GetBinEntries(h01_[ism-1]->GetBin(i));
        if ( num01 >= n_min_bin ) {
          mean01 = h01_[ism-1]->GetBinContent(h01_[ism-1]->GetBin(i));
          rms01  = h01_[ism-1]->GetBinError(h01_[ism-1]->GetBin(i));
          update_channel = true;
        }
      }

      if ( h02_[ism-1] && h02_[ism-1]->GetEntries() >= n_min_tot ) {
        num02 = h02_[ism-1]->GetBinEntries(h02_[ism-1]->GetBin(i));
        if ( num02 >= n_min_bin ) {
          mean02 = h02_[ism-1]->GetBinContent(h02_[ism-1]->GetBin(i));
          rms02  = h02_[ism-1]->GetBinError(h02_[ism-1]->GetBin(i));
          update_channel = true;
        }
      }

      if ( h03_[ism-1] && h03_[ism-1]->GetEntries() >= n_min_tot ) {
        num03 = h03_[ism-1]->GetBinEntries(h03_[ism-1]->GetBin(i));
        if ( num03 >= n_min_bin ) {
          mean03 = h03_[ism-1]->GetBinContent(h03_[ism-1]->GetBin(i));
          rms03  = h03_[ism-1]->GetBinError(h03_[ism-1]->GetBin(i));
          update_channel = true;
        }
      }

      if ( h04_[ism-1] && h04_[ism-1]->GetEntries() >= n_min_tot ) {
        num04 = h04_[ism-1]->GetBinEntries(h04_[ism-1]->GetBin(i));
        if ( num04 >= n_min_bin ) {
          mean04 = h04_[ism-1]->GetBinContent(h04_[ism-1]->GetBin(i));
          rms04  = h04_[ism-1]->GetBinError(h04_[ism-1]->GetBin(i));
          update_channel = true;
        }
      }

      if ( update_channel ) {

        if ( i == 1 ) {

          cout << "Inserting dataset for SM=" << ism << endl;

          cout << "PNs (" << i << ") L1 " << num01  << " "
                                          << mean01 << " "
                                          << rms01  << endl;
          cout << "PNs (" << i << ") L2 " << num02  << " "
                                          << mean02 << " "
                                          << rms02  << endl;
          cout << "PNs (" << i << ") L3 " << num03  << " "
                                          << mean03 << " "
                                          << rms03  << endl;
          cout << "PNs (" << i << ") L4 " << num04  << " "
                                          << mean04 << " "
                                          << rms04  << endl;
        }

        p.setADCMean(mean01);
        p.setADCRMS(rms01);

        p.setTaskStatus(1);

        if ( econn ) {
          try {
            ecid = econn->getEcalLogicID("EB_LM_PN", ism, i-1);
            dataset[ecid] = p;
          } catch (runtime_error &e) {
            cerr << e.what() << endl;
          }
        }

      }

    }

  }

  if ( econn ) {
    try {
      cout << "Inserting dataset ... " << flush;
      econn->insertDataSet(&dataset, runiov, runtag );
      cout << "done." << endl;
    } catch (runtime_error &e) {
      cerr << e.what() << endl;
    }
  }

}

void EBPnDiodeClient::subscribe(void){

  // subscribe to all monitorable matching pattern
  mui_->subscribe("*/EcalBarrel/EBPnDiodeTask/Laser1/EBPT PNs SM*");
  mui_->subscribe("*/EcalBarrel/EBPnDiodeTask/Laser2/EBPT PNs SM*");
  mui_->subscribe("*/EcalBarrel/EBPnDiodeTask/Laser3/EBPT PNs SM*");
  mui_->subscribe("*/EcalBarrel/EBPnDiodeTask/Laser4/EBPT PNs SM*");

  Char_t histo[80];

  for ( int ism = 1; ism <= 36; ism++ ) {

    sprintf(histo, "EBPT PNs SM%02d L1", ism);
    me_h01_[ism-1] = mui_->collateProf2D(histo, histo, "EcalBarrel/Sums/EBPnDiodeTask/Laser1");
    sprintf(histo, "*/EcalBarrel/EBPnDiodeTask/Laser1/EBPT PNs SM%02d L1", ism);
    mui_->add(me_h01_[ism-1], histo);

    sprintf(histo, "EBPT PNs SM%02d L2", ism);
    me_h02_[ism-1] = mui_->collateProf2D(histo, histo, "EcalBarrel/Sums/EBPnDiodeTask/Laser2");
    sprintf(histo, "*/EcalBarrel/EBPnDiodeTask/Laser2/EBPT PNs SM%02d L2", ism);
    mui_->add(me_h02_[ism-1], histo);

    sprintf(histo, "EBPT PNs SM%02d L3", ism);
    me_h03_[ism-1] = mui_->collateProf2D(histo, histo, "EcalBarrel/Sums/EBPnDiodeTask/Laser3");
    sprintf(histo, "*/EcalBarrel/EBPnDiodeTask/Laser3/EBPT PNs SM%02d L3", ism);
    mui_->add(me_h03_[ism-1], histo);

    sprintf(histo, "EBPT PNs SM%02d L4", ism);
    me_h04_[ism-1] = mui_->collateProf2D(histo, histo, "EcalBarrel/Sums/EBPnDiodeTask/Laser4");
    sprintf(histo, "*/EcalBarrel/EBPnDiodeTask/Laser4/EBPT PNs SM%02d L4", ism);
    mui_->add(me_h04_[ism-1], histo);

  }

}

void EBPnDiodeClient::subscribeNew(void){

  // subscribe to new monitorable matching pattern
  mui_->subscribeNew("*/EcalBarrel/EBPnDiodeTask/Laser1/EBPT PNs SM*");
  mui_->subscribeNew("*/EcalBarrel/EBPnDiodeTask/Laser2/EBPT PNs SM*");
  mui_->subscribeNew("*/EcalBarrel/EBPnDiodeTask/Laser3/EBPT PNs SM*");
  mui_->subscribeNew("*/EcalBarrel/EBPnDiodeTask/Laser4/EBPT PNs SM*");

}

void EBPnDiodeClient::unsubscribe(void){

  // unsubscribe to all monitorable matching pattern
  mui_->unsubscribe("*/EcalBarrel/EBPnDiodeTask/Laser1/EBPT PNs SM*");
  mui_->unsubscribe("*/EcalBarrel/EBPnDiodeTask/Laser2/EBPT PNs SM*");
  mui_->unsubscribe("*/EcalBarrel/EBPnDiodeTask/Laser3/EBPT PNs SM*");
  mui_->unsubscribe("*/EcalBarrel/EBPnDiodeTask/Laser4/EBPT PNs SM*");

}

void EBPnDiodeClient::analyze(const edm::Event& e, const edm::EventSetup& c){

  ievt_++;
  jevt_++;
  if ( ievt_ % 10 == 0 )  
    cout << "EBPnDiodeClient: ievt/jevt = " << ievt_ << "/" << jevt_ << endl;

  Char_t histo[150];

  MonitorElement* me;
  MonitorElementT<TNamed>* ob;

  for ( int ism = 1; ism <= 36; ism++ ) {

//    sprintf(histo, "Collector/FU0/EcalBarrel/EBPnDiodeTask/Laser1/EBPT PNs SM%02d L1", ism);
    sprintf(histo, "EcalBarrel/Sums/EBPnDiodeTask/Laser1/EBPT PNs SM%02d L1", ism);
    me = mui_->get(histo);
    if ( me ) {
      cout << "Found '" << histo << "'" << endl;
      ob = dynamic_cast<MonitorElementT<TNamed>*> (me);
      if ( ob ) {
        if ( h01_[ism-1] ) delete h01_[ism-1];
        sprintf(histo, "ME EBPT PNs SM%02d L1", ism);
        h01_[ism-1] = dynamic_cast<TProfile*> ((ob->operator->())->Clone(histo));
      }
    }

//    sprintf(histo, "Collector/FU0/EcalBarrel/EBPnDiodeTask/Laser2/EBPT PNs SM%02d L2", ism);
    sprintf(histo, "EcalBarrel/Sums/EBPnDiodeTask/Laser2/EBPT PNs SM%02d L2", ism);
    me = mui_->get(histo);
    if ( me ) {
      cout << "Found '" << histo << "'" << endl;
      ob = dynamic_cast<MonitorElementT<TNamed>*> (me);
      if ( ob ) {
        if ( h02_[ism-1] ) delete h02_[ism-1];
        sprintf(histo, "ME EBPT PNs SM%02d L2", ism);
        h02_[ism-1] = dynamic_cast<TProfile*> ((ob->operator->())->Clone(histo));
      }
    }

//    sprintf(histo, "Collector/FU0/EcalBarrel/EBPnDiodeTask/Laser3/EBPT PNs SM%02d L3", ism);
    sprintf(histo, "EcalBarrel/Sums/EBPnDiodeTask/Laser3/EBPT PNs SM%02d L3", ism);
    me = mui_->get(histo);
    if ( me ) {
      cout << "Found '" << histo << "'" << endl;
      ob = dynamic_cast<MonitorElementT<TNamed>*> (me);
      if ( ob ) {
        if ( h03_[ism-1] ) delete h03_[ism-1];
        sprintf(histo, "ME EBPT PNs SM%02d L3", ism);
        h03_[ism-1] = dynamic_cast<TProfile*> ((ob->operator->())->Clone(histo));
      }
    }

//    sprintf(histo, "Collector/FU0/EcalBarrel/EBPnDiodeTask/Laser4/EBPT PNs SM%02d L4", ism);
    sprintf(histo, "EcalBarrel/Sums/EBPnDiodeTask/Laser4/EBPT PNs SM%02d L4", ism);
    me = mui_->get(histo);
    if ( me ) {
      cout << "Found '" << histo << "'" << endl;
      ob = dynamic_cast<MonitorElementT<TNamed>*> (me);
      if ( ob ) {
        if ( h04_[ism-1] ) delete h04_[ism-1];
        sprintf(histo, "ME EBPT PNs SM%02d L4", ism);
        h04_[ism-1] = dynamic_cast<TProfile*> ((ob->operator->())->Clone(histo));
      }
    }

  }

}

void EBPnDiodeClient::htmlOutput(int run, string htmlDir, string htmlName){

  cout << "Preparing EBPnDiodeClient html output ..." << endl;

  ofstream htmlFile;

  htmlFile.open((htmlDir + htmlName).c_str());

  // html page header
  htmlFile << "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">  " << endl;
  htmlFile << "<html>  " << endl;
  htmlFile << "<head>  " << endl;
  htmlFile << "  <meta content=\"text/html; charset=ISO-8859-1\"  " << endl;
  htmlFile << " http-equiv=\"content-type\">  " << endl;
  htmlFile << "  <title>Monitor:PnDiodeTask output</title> " << endl;
  htmlFile << "</head>  " << endl;
  htmlFile << "<style type=\"text/css\"> td { font-weight: bold } </style>" << endl;
  htmlFile << "<body>  " << endl;
  htmlFile << "<br>  " << endl;
  htmlFile << "<h2>Run:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;" << endl;
  htmlFile << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <span " << endl; 
  htmlFile << " style=\"color: rgb(0, 0, 153);\">" << run << "</span></h2>" << endl;
  htmlFile << "<h2>Monitoring task:&nbsp;&nbsp;&nbsp;&nbsp; <span " << endl;
  htmlFile << " style=\"color: rgb(0, 0, 153);\">PNDIODE</span></h2> " << endl;
  htmlFile << "<hr>" << endl;
  htmlFile << "<table border=1><tr><td bgcolor=red>channel has problems in this task</td>" << endl;
  htmlFile << "<td bgcolor=lime>channel has NO problems</td>" << endl;
  htmlFile << "<td bgcolor=white>channel is missing</td></table>" << endl;
  htmlFile << "<hr>" << endl;

  // Produce the plots to be shown as .jpg files from existing histograms

  int csize = 250;

//  double histMax = 1.e15;

  string imgNameME[2], imgName, meName;

  // Loop on barrel supermodules

  for ( int ism = 1 ; ism <= 36 ; ism++ ) {

    if ( h01_[ism-1] && h02_[ism-1] && h03_[ism-1] && h04_[ism-1] ) {

      // Loop on wavelenght

      for ( int iCanvas=1 ; iCanvas <= 2 ; iCanvas++ ) {

        // Monitoring elements plots

        TProfile* objp = 0;

        switch ( iCanvas ) { 
        case 1:
          meName = h01_[ism-1]->GetName();
          objp = h01_[ism-1];
          break;
        case 2:
          meName = h02_[ism-1]->GetName();
          objp = h02_[ism-1];
          break;
        case 3:
          meName = h03_[ism-1]->GetName();
          objp = h03_[ism-1];
          break;
        case 4:
          meName = h04_[ism-1]->GetName();
          objp = h04_[ism-1];
          break;
        }

        TCanvas *cAmp = new TCanvas("cAmp" , "Temp", csize , csize );
        for ( unsigned int iAmp=0 ; iAmp < meName.size(); iAmp++ ) {
          if ( meName.substr(iAmp,1) == " " )  {
            meName.replace(iAmp, 1 ,"_" );
          }
        }
        imgNameME[iCanvas-1] = meName + ".jpg";
        imgName = htmlDir + imgNameME[iCanvas-1];
        gStyle->SetOptStat("euomr");
        objp->SetStats(kTRUE);
//        if ( objp->GetMaximum(histMax) > 0. ) {
//          gPad->SetLogy(1);
//        } else {
//          gPad->SetLogy(0);
//        }
        objp->Draw();
        cAmp->Update();
        TPaveStats* stAmp = dynamic_cast<TPaveStats*>(objp->FindObject("stats"));
        if ( stAmp ) {
          stAmp->SetX1NDC(0.6);
          stAmp->SetY1NDC(0.75);
        }
        cAmp->SaveAs(imgName.c_str());
        gPad->SetLogy(0);
        delete cAmp;

      }
    }

    htmlFile << "<h3><strong>Supermodule&nbsp;&nbsp;" << ism << "</strong></h3>" << endl;
    htmlFile << "<table border=\"0\" cellspacing=\"0\" " << endl;
    htmlFile << "cellpadding=\"10\" align=\"center\"> " << endl;
    htmlFile << "<tr align=\"center\">" << endl;

    for ( int iCanvas = 1 ; iCanvas <= 2 ; iCanvas++ ) {

      if ( imgNameME[iCanvas-1].size() != 0 )
        htmlFile << "<td colspan=\"2\"><img src=\"" << imgNameME[iCanvas-1] << "\"></td>" << endl;
      else
        htmlFile << "<td><img src=\"" << " " << "\"></td>" << endl;

    }
    htmlFile << "</tr>" << endl;
    htmlFile << "</table>" << endl;
    htmlFile << "<br>" << endl;

  }

  // html page footer
  htmlFile << "</body> " << endl;
  htmlFile << "</html> " << endl;

  htmlFile.close();

}

