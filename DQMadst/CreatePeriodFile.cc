// C++
#include <iostream> // required for cout etc
#include <fstream>  // for reading in from / writing to an external file
#include <sstream> // string manipulation
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>

// ROOT 
#include <TMath.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TGraph.h>
#include <TGraphErrors.h>

//ADST includes 
#include "RecEvent.h"
#include "RecEventFile.h"
//#include "RdEvent.h"

using namespace std;

const double PI_rad = TMath::Pi();
const double PI_deg = 180.;
const double rad_to_deg = PI_deg/PI_rad;
const double deg_to_rad = PI_rad/PI_deg;
const int NFDeyes = 6;
const int NSDlogErange = 13;
const int NSDhIndexes = 2;

const int NOINDEX = 1000;


const double SDlogElimits[NSDlogErange] = {0., 17.8, 18.0, 18.2, 18.4, 18.6, 18.8, 19.0, 19.2, 19.4, 19.6, 19.8, 21.0};
const string SDlogErange[NSDlogErange] = {"All energies","17.8 - 18.0","18.0 - 18.2","18.2 - 18.4",
					  "18.4 - 18.6","18.6 - 18.8","18.8 - 19.0","19.0 - 19.2",
					  "19.2 - 19.4","19.4 - 19.6","19.6 - 19.8", "19.8 - max", "dummy"};
const string FDeyes[NFDeyes]={"All eyes","Los Leones","Los Morados","Loma Amarilla","Coihueco","Heat"};

void usage();
TH1F *CreateHistogram1D(string stream, int index, string pointer_title, string h_title, 
			int NBinX, double lowX, double highX, string titleX);
TH2F *CreateHistogram2D(string stream, int index, string pointer_title, string h_title, 
			int NBinX, double lowX, double highX, string titleX,
			int NBinY, double lowY, double highY, string titleY);
TGraphErrors *CreateGraphErrors(string stream, int index, string pointer_title, string h_title);
TH1F *FillHistogram1D(string stream, int index, string pointer_title, double valX);
TH2F *FillHistogram2D(string stream, int index, string pointer_title, double valX, double valY);
TGraphErrors *FillLDFGraphErrors(string stream, int index, string pointer_title, const vector<SdRecStation> & sdstations, double zenith_angle);
void WriteHistogram1D(TDirectory *dir, string stream, int index, string pointer_title);
void WriteHistogram2D(TDirectory *dir, string stream, int index, string pointer_title);
void WriteLDFGraphErrors(TDirectory *dir, string stream, int index, string pointer_title);
//TGraphErrors *FillLDFGraph(TGraphErrors *g, const vector<SdRecStation> & sdstations, double zenith_angle);
TGraphErrors *ApplyLDFGraphErrorsStyle(TGraphErrors *ge);
double ApplyCICcorrection(double signal, double zenith_angle);

int  main(int argc, char **argv) {
  
  if (argc < 2) {
    usage(); // see usage() at the bottom of this program for usage details
    return 0;
  }

  stringstream pointer;
  stringstream title;
  TH1F *h1d;
  TH2F *h2d;
  TGraphErrors *ge;

  int SDhIndexes[NSDhIndexes];
  int Nindex = 0;
  int index;

  // Include here SD related plots - sd: all tanks
  // sd0: all energies / sd<1-11>: energy ranges from log(E/eV)=[17.8-18.0] to Log(E/eV)=[19.8-max]
  for (int l=0;l<NSDlogErange-1;l++) {
    // 1D: Tank multiplicity
    h1d = CreateHistogram1D("sd",l,"Multiplicity","Multiplicity",100,0.,100.,"Multiplicity");
    // 1D: Zenith angle
    h1d = CreateHistogram1D("sd",l,"Zenith","Zenith (#theta)",90,0.,90.,"#theta (deg)");
    // 1D: Sin squared Zenith angle
    h1d = CreateHistogram1D("sd",l,"SinSqZen","Zenith Sin^{2}(#theta)",100,0.,1.,"Sin^{2}(#theta)");
    // 1D: Azimuth
    h1d = CreateHistogram1D("sd",l,"Azimuth","Azimuth (#phi)",120,0.,360.,"#phi (deg)");
    // 1D: Log S1000
    h1d = CreateHistogram1D("sd",l,"LogS1000","Log_{10}(S1000)",100,0.,3.5,"Log_{10}(S1000)");
    // 1D: Log(E/eV)
    h1d = CreateHistogram1D("sd",l,"LogE","Log_{10}(E/eV)",100,17.,21.,"Log_{10}(E/eV)");
    // 2D: Core location
    h2d = CreateHistogram2D("sd",l,"CoreLocation","SD Core location",150,-40.,35.,"km",150,-35.,40.,"km");
    // 2D: Skymap (galactic coordinates)
    h2d = CreateHistogram2D("sd",l,"Skymap","Exposure",180,-180.,180.,"Galactic Lon (deg)",90,-90.,90.,"Galactic Lat (deg)");
    // Graph: average LDFs
    ge = CreateGraphErrors("sd",l,"MeanLDF","Mean LDF");
  }

  // Include here FD related plots:
  // fd0: all eyes / fd1-4: (1) Los Leones / (2) Los Morados / (3) Loma Amarilla / (4) Coihueco / (5) Heat
  for (int eye=0;eye<NFDeyes;eye++) {
    // 1D: Sin squared of Zenith angle
    h1d = CreateHistogram1D("fd",eye,"SinSqZen","Zenith Sin^{2}(#theta)",100,0.,1.,"Sin^{2}(#theta)");
    // 1D: Azimuth angle
    h1d = CreateHistogram1D("fd",eye,"Azimuth","Azimuth (#phi)",120,0.,360.,"#phi (deg)");
    // 1D: Xmax
    h1d = CreateHistogram1D("fd",eye,"Xmax","X_{max}",120,200.,1400.,"X_{max} (g/cm^{2})");
    // 1D: DeltaXmax
    h1d = CreateHistogram1D("fd",eye,"DeltaXmax","#DeltaX_{max}",100,0.,200.,"#DeltaX_{max} (g/cm^{2})");
    // 1D: Log10 E
    h1d = CreateHistogram1D("fd",eye,"LogE","Log_{10}(E/eV)",100,17.,21.,"Log_{10}(E/eV)");
    // 2D: DeltaXmax vs Xmax
    h2d = CreateHistogram2D("fd",eye,"DeltaVSXmax","#DeltaX_{max} vs X_{max}",120,200.,1400.,"X_{max} (g/cm^{2})",100,0.,200.,"#DeltaX_{max} (g/cm^{2})");
  }

  // Include here Hybrid related plots:
  // hy0: all eyes / hy1-4: (1) Los Leones / (2) Los Morados / (3) Loma Amarilla / (4) Coihueco / (5) Heat
  for (int eye=0;eye<NFDeyes;eye++) {
    // 1D: SD-FD timing
    h1d = CreateHistogram1D("hy",eye,"SdFdTime","SD-FD timing",80,-200.,200.,"SD-FD time (ns)");
    // 2D: SD vs FD log10 E
    h2d = CreateHistogram2D("hy",eye,"SDvsFDLogE","Log_{10}(E/eV) SD vs FD",100,17.,21.,"FD Log_{10}(E/eV)",100,17.,21.,"SD Log_{10}(E/eV)");
  }
  
  /*
  // Include here Radio related plots:
  // 1D: Cos Zenith angle
  h1d = CreateHistogram1D("rd",NOINDEX,"CosZenith","Cos Zenith (#theta)",100,0.,1.,"cos(#theta)");
  // 1D: Azimuth
  h1d = CreateHistogram1D("rd",NOINDEX,"Azimuth","Azimuth (#phi)",120,0.,360.,"#phi (deg)");
  */

  cout << endl;

  // Loop over files - read the file from the second to last argument

  stringstream filess;
  for (int iFile = 1; iFile <= argc - 1; iFile++) {

    RecEventFile dataFile(argv[iFile]);
    RecEvent* theRecEvent = new RecEvent();
    dataFile.SetBuffers(&(theRecEvent));

    unsigned int ntotThisFile = dataFile.GetNEvents();
   
    if (iFile == 1) {
      filess << "From_" << dataFile.GetActiveFileName().substr(dataFile.GetActiveFileName().size() - 15, 10) << "_To_";
    }
    else if (iFile == argc - 1) {
      filess << dataFile.GetActiveFileName().substr(dataFile.GetActiveFileName().size() - 15, 10) << ".root";
    }
    cout << "reading file " << dataFile.GetActiveFileName() << " with " << ntotThisFile << " events." << endl;

    // ----------------------------------------------------------------------
    // loop over all the events in the file
    // Try to avoid iterating over simply "i", it can be ambiguous.
    for (unsigned int iEvent = 0; iEvent < ntotThisFile; iEvent++) {

      // if there are no more events left in this file, move on the the next file
      if ((!dataFile.ReadEvent(iEvent)) == RecEventFile::eSuccess) continue;

      // SD event
      const SDEvent& sevent = theRecEvent->GetSDEvent();
      const int eventID = sevent.GetEventId();
      const SdRecShower& sdevent = sevent.GetSdRecShower();
      const vector<SdRecStation> & sdstations    = sevent.GetStationVector();

      // Insert here needed SD-related variables 
      const int vSD_Multiplicity = sdstations.size();
      const double vSD_Zenith_deg = sdevent.GetZenith() * rad_to_deg;
      const double vSD_Azimuth_deg = sdevent.GetAzimuth() * rad_to_deg; 
      const double vSD_SinsqZenith = 1. - pow(sdevent.GetCosZenith(),2);
      const double vSD_S1000 = sdevent.GetS1000();
      const double vSD_LogS1000 = log10(vSD_S1000);
      const double vSD_E = sdevent.GetEnergy();
      const double vSD_LogE = log10(vSD_E);
      const double vSD_RA = sdevent.GetRightAscension()-PI_rad;
      const double vSD_Dec = sdevent.GetDeclination();
      const double vSD_LonGal = (sdevent.GetGalacticLongitude()-PI_rad) * rad_to_deg;
      const double vSD_LatGal = sdevent.GetGalacticLatitude() * rad_to_deg;
      const TVector3& vSD_CoreLocation = sdevent.GetCoreSiteCS(); 

      /*
      // RD event
      const RdEvent& revent = theRecEvent->GetRdEvent();
      const RdRecShower& rdevent = revent.GetRdRecShower();

      // Insert here needed RD-related variables 
      const double vRD_cosZenith = rdevent.GetCosZenith();
      const double vRD_Azimuth_deg = rdevent.GetAzimuth() * rad_to_deg;
      */

      // Filling histograms SD data
      for (int l=0;l<NSDlogErange-1;l++) {
	if (vSD_LogE>=SDlogElimits[l] && vSD_LogE<SDlogElimits[l+1]) {
	  SDhIndexes[0] = 0; // always fill sd0
	  SDhIndexes[1] = l; // if l>0 then need to fill sd<l> as well
	  if (l==0)
	    Nindex = 1; // if l==0, only fill sd0
	  else
	    Nindex = 2; // if l>0, fill sd0 and sd<l>
	}
      }

      for (int i=0;i<Nindex;i++) {
	if (sevent.GetRecLevel() >= eHasSdAxis) {
	  // 1D: Tank multiplicity
	  h1d = FillHistogram1D("sd",SDhIndexes[i],"Multiplicity",vSD_Multiplicity);
	  // 1D: Zenith angle
	  h1d = FillHistogram1D("sd",SDhIndexes[i],"Zenith",vSD_Zenith_deg);
	  // 1D: Sin squared Zenith angle
	  h1d = FillHistogram1D("sd",SDhIndexes[i],"SinSqZen",vSD_SinsqZenith);
	  // 1D: Azimuth
	  h1d = FillHistogram1D("sd",SDhIndexes[i],"Azimuth",vSD_Azimuth_deg);
	  // 1D: Log S1000
	  h1d = FillHistogram1D("sd",SDhIndexes[i],"LogS1000",vSD_LogS1000);
	  // 1D: Log E
	  h1d = FillHistogram1D("sd",SDhIndexes[i],"LogE",vSD_LogE);
	  // 2D: Core location
	  h2d = FillHistogram2D("sd",SDhIndexes[i],"CoreLocation",vSD_CoreLocation[0]/1000.,vSD_CoreLocation[1]/1000.);
	  // 2D: Skymap
	  h2d = FillHistogram2D("sd",SDhIndexes[i],"Skymap",vSD_RA*rad_to_deg,vSD_Dec*rad_to_deg);
	  
	  if (Nindex==2) {
	    index = 1;
	    if ( (sevent.GetRecLevel() >= eHasLDF) && (vSD_Zenith_deg<60.) )
	      ge = FillLDFGraphErrors("sd",SDhIndexes[index],"MeanLDF",sdstations,sdevent.GetZenith());
	  }
	}
      }
 
      /*
      // Filling histograms RD data
      if (revent.GetRdRecLevel() >= eHasRdShower) {
	// 1D: Cos Zenith angle
	h1d = FillHistogram1D("rd",NOINDEX,"CosZenith",vRD_cosZenith);
	// 1D: Azimuth
	h1d = FillHistogram1D("rd",NOINDEX,"Azimuth",vRD_Azimuth_deg);
      }
      */

      // ----------------------------------------------------------------------
      // Loop over eyes
      std::vector<FDEvent> & fdEvents = theRecEvent->GetFDEvents();
      for (vector<FDEvent>::iterator eye = fdEvents.begin(); eye != fdEvents.end(); ++eye) {  
	// eye is an FDEvent object

	int eyeID=eye->GetEyeId();
   
	const FdRecShower &recShower = eye->GetFdRecShower();
	const FdRecGeometry &recGeometry = eye->GetFdRecGeometry();

	// Insert here needed FD-related variables 

	const double vFD_E = recShower.GetEnergy();
	const double vFD_LogE = log10(vFD_E);
	const double vFD_Xmax = recShower.GetXmax();
	const double vFD_DeltaXmax = recShower.GetXmaxError();
	const double vFD_Zenith_rad = recShower.GetZenith(); 
	const double vFD_Zenith_deg = vFD_Zenith_rad * rad_to_deg;
	const double vFD_Azimuth_rad = recShower.GetAzimuth(); 
	const double vFD_Azimuth_deg = vFD_Azimuth_rad * rad_to_deg;
	const double vFD_CosZenith = cos(vFD_Zenith_rad);
	const double vFD_SinsqZenith = pow(sin(vFD_Zenith_rad),2);
	const double vFD_Tracklength = recShower.GetTrackLength();
	const double vHY_SdFdTime = recGeometry.GetSDFDTimeOffset();

	Nindex = 2;
	int FDeyeIDs[2] = {0,eyeID}; // because I need to fill fd0 or hy0 as well / quick and dirty way to do this!

	for (int i=0;i<Nindex;i++) {

	  // Filling histograms FD data
	  if (recGeometry.GetGeomRecLevel() >= eMonoGeometry) {
	    // 1D: Sin squared of zenith angle
	    h1d = FillHistogram1D("fd",FDeyeIDs[i],"SinSqZen",vFD_SinsqZenith);
	    // 1D: Azimuth angle
	    h1d = FillHistogram1D("fd",FDeyeIDs[i],"Azimuth",vFD_Azimuth_deg);
	  }
	  if (eye->GetRecLevel()>= eHasGHParameters) {
	    // 1D: Xmax
	    h1d = FillHistogram1D("fd",FDeyeIDs[i],"Xmax",vFD_Xmax);
	    // 1D: DeltaXmax
	    h1d = FillHistogram1D("fd",FDeyeIDs[i],"DeltaXmax",vFD_DeltaXmax);
	    // 2D: DeltaXmax vs Xmax
	    h2d = FillHistogram2D("fd",FDeyeIDs[i],"DeltaVSXmax",vFD_Xmax,vFD_DeltaXmax);	    
	    if (eye->GetRecLevel() == eHasEnergy) {
	      // 1D: Log10 E
	      h1d = FillHistogram1D("fd",FDeyeIDs[i],"LogE",vFD_LogE);
	    }
	  }

	  // Filling histograms HY data
	  // 1D: SD - FD timing
	  h1d = FillHistogram1D("hy",FDeyeIDs[i],"SdFdTime",vHY_SdFdTime);
	  if (eye->GetRecLevel() == eHasEnergy) {
	    // 2D: SD vs FD log10 E
	    h2d = FillHistogram2D("hy",FDeyeIDs[i],"SDvsFDLogE",vFD_LogE,vSD_LogE);
	  }

	}

      } // eyes

    } // events in file
	
  } // files	

  TDirectory *previous = (TDirectory*)gDirectory;

  TFile *f = new TFile(filess.str().c_str(),"recreate");

  // Saving SD histograms & graphs
  for (int l=0;l<NSDlogErange-1;l++) {
    // 1D: Multiplicity
    WriteHistogram1D(previous,"sd",l,"Multiplicity");
    // 1D: Zenith angle
    WriteHistogram1D(previous,"sd",l,"Zenith");
    // 1D: Sin squared Zenith angle
    WriteHistogram1D(previous,"sd",l,"SinSqZen");
    // 1D: Azimuth
    WriteHistogram1D(previous,"sd",l,"Azimuth");
    // 1D: Log S1000
    WriteHistogram1D(previous,"sd",l,"LogS1000");
    // 1D: Log E
    WriteHistogram1D(previous,"sd",l,"LogE");
    // 2D: Core location
    WriteHistogram2D(previous,"sd",l,"CoreLocation");
    // 2D: Skymap
    WriteHistogram2D(previous,"sd",l,"Skymap");
    // Graph: average LDF graphs
    WriteLDFGraphErrors(previous,"sd",l,"MeanLDF");
  }

  /*
  // Saving RD histograms & graphs
  // 1D: Cos Zenith angle
  WriteHistogram1D(previous,"rd",NOINDEX,"CosZenith");
  // 1D: Azimuth
  WriteHistogram1D(previous,"rd",NOINDEX,"Azimuth");
  */

  // Saving FD histograms
  for (int e=0;e<NFDeyes;e++) {
    // 1D: Sin squared of zenith angle
    WriteHistogram1D(previous,"fd",e,"SinSqZen");
    // 1D: Azimuth angle
    WriteHistogram1D(previous,"fd",e,"Azimuth");
    // 1D: Xmax
    WriteHistogram1D(previous,"fd",e,"Xmax");
    // 1D: DeltaXmax
    WriteHistogram1D(previous,"fd",e,"DeltaXmax");
    // 1D: Log10 E
    WriteHistogram1D(previous,"fd",e,"LogE");
    // 2D: DeltaXmax vs Xmax
    WriteHistogram2D(previous,"fd",e,"DeltaVSXmax");
  }
    
  // Saving HY histograms
  for (int e=0;e<NFDeyes;e++) {
    // 1D: SD-FD timing
    WriteHistogram1D(previous,"hy",e,"SdFdTime");
    // 2D: SD vs FD log10 E
    WriteHistogram2D(previous,"hy",e,"SDvsFDLogE");
  }    

  f->Close();

  cout << endl << "Root file period saved under: " << filess.str() << endl << endl;
  
  return 0;
}

TH1F *CreateHistogram1D(string stream, int index, string pointer_title, string h_title, 
			int NBinX, double lowX, double highX, string titleX)
{
  stringstream pointer;
  stringstream title;
  TH1F *h1d;
  pointer << stream << index << "_" << pointer_title;
  if (stream=="sd")
    title << h_title << " - Log(E/eV) = [" << SDlogErange[index] << "]";
  if ( (stream=="fd") || (stream=="hy") )
     title << h_title << " - " << FDeyes[index];
  h1d = new TH1F(pointer.str().c_str(),title.str().c_str(),NBinX,lowX,highX);
  h1d->GetXaxis()->SetTitle(titleX.c_str());
  return h1d;
}

TH2F *CreateHistogram2D(string stream, int index, string pointer_title, string h_title, 
			int NBinX, double lowX, double highX, string titleX,
			int NBinY, double lowY, double highY, string titleY) 
{
  stringstream pointer;
  stringstream title;
  TH2F *h2d;
  pointer << stream << index << "_" << pointer_title;
  if (stream=="sd")
    title << h_title << " - Log(E/eV) = [" << SDlogErange[index] << "]";
  if ( (stream=="fd") || (stream=="hy") )
     title << h_title << " - " << FDeyes[index];
  h2d = new TH2F(pointer.str().c_str(),title.str().c_str(),NBinX,lowX,highX,NBinY,lowY,highY);
  h2d->GetXaxis()->SetTitle(titleX.c_str());
  h2d->GetYaxis()->SetTitle(titleY.c_str());
  return h2d;
}

TGraphErrors *CreateGraphErrors(string stream, int index, string pointer_title, string h_title)
{
  stringstream pointer;
  stringstream title;
  TGraphErrors *ge;
  pointer << stream << index << "_" << pointer_title;
  if (stream=="sd")
    title << h_title << " - Log(E/eV) = [" << SDlogErange[index] << "]";
  if ( (stream=="fd") || (stream=="hy") )
     title << h_title << " - " << FDeyes[index];
  ge = new TGraphErrors();
  ge->SetName(pointer.str().c_str());
  ge->SetTitle(title.str().c_str());
  gDirectory->GetList()->Add(ge);
  return ge;
}

TH1F *FillHistogram1D(string stream, int index, string pointer_title, double valX)
{
  stringstream pointer;
  stringstream title;
  TH1F *h1d;

  if (index==NOINDEX)
    pointer << stream << "_" << pointer_title;
  else
    pointer << stream << index << "_" << pointer_title;

  h1d = (TH1F*)gDirectory->Get(pointer.str().c_str());
  h1d->Fill(valX);
  return h1d;
}

TH2F *FillHistogram2D(string stream, int index, string pointer_title, double valX, double valY)
{
  stringstream pointer;
  stringstream title;
  TH2F *h2d;

  if (index==NOINDEX)
    pointer << stream << "_" << pointer_title;
  else
    pointer << stream << index << "_" << pointer_title;

  h2d = (TH2F*)gDirectory->Get(pointer.str().c_str());
  h2d->Fill(valX,valY);
  return h2d;
}

TGraphErrors *FillLDFGraphErrors(string stream, int index, string pointer_title, const vector<SdRecStation> & sdstations, double zenith_angle)
{
  stringstream pointer;
  stringstream title;
  TGraphErrors *ge;

  double Signal, SignalError;
  double SignalCIC, SignalErrorCIC;
  int Npoints_so_far; 
  int Counter = 0;

  pointer << stream << index << "_" << pointer_title;
  ge = (TGraphErrors*)gDirectory->Get(pointer.str().c_str());
  Npoints_so_far = ge->GetN();

  for (int i=0;i<sdstations.size();i++) {
    if (!(sdstations[i].IsLowGainSaturated()) && 
	(!(sdstations[i].IsRandom())) && 
	(!(sdstations[i].IsAccidental())) ) {
      Signal = sdstations[i].GetTotalSignal();
      SignalError = sdstations[i].GetTotalSignalError();
      SignalCIC = ApplyCICcorrection(Signal,zenith_angle);
      SignalErrorCIC = ApplyCICcorrection(SignalError,zenith_angle); // not propagating error of CIC.
      if ( (Signal-2*SignalError) > 0.) {
	ge->SetPoint(Npoints_so_far+Counter,sdstations[i].GetSPDistance(),SignalCIC);
	ge->SetPointError(Npoints_so_far+Counter,0.,SignalErrorCIC);
	Counter++;
      }
    }
  }

  return ge;
}

void WriteHistogram1D(TDirectory *dir, string stream, int index, string pointer_title)
{
  stringstream pointer;
  TH1F *h1d;

  if (index==NOINDEX)
    pointer << stream << "_" << pointer_title;
  else
    pointer << stream << index << "_" << pointer_title;

  h1d = (TH1F*)dir->Get(pointer.str().c_str());
  h1d->Write();
}

void WriteHistogram2D(TDirectory *dir, string stream, int index, string pointer_title)
{
  stringstream pointer;
  TH2F *h2d;

  if (index==NOINDEX)
    pointer << stream << "_" << pointer_title;
  else
    pointer << stream << index << "_" << pointer_title;

  h2d = (TH2F*)dir->Get(pointer.str().c_str());
  h2d->Write();
}

void WriteLDFGraphErrors(TDirectory *dir, string stream, int index, string pointer_title)
{
  stringstream pointer;
  TGraphErrors *ge;
  pointer << stream << index << "_" << pointer_title;
  ge = (TGraphErrors*)dir->Get(pointer.str().c_str());
  ge = ApplyLDFGraphErrorsStyle(ge);
  ge->Write();
}

TGraphErrors *ApplyLDFGraphErrorsStyle(TGraphErrors *ge)
{
  ge->SetMarkerStyle(20);
  ge->SetMarkerSize(0.05);
  ge->SetLineWidth(1.);
  ge->GetXaxis()->SetRangeUser(0.,4000.);
  ge->GetXaxis()->SetTitle("r (m)");
  ge->GetYaxis()->SetTitle("S (au)");
  return ge;
}

double ApplyCICcorrection(double signal, double zenith_angle) {
  // Based on ICRC 2013
  double a = 0.98;
  double b = -1.68;
  double c = -1.3;
  double x = TMath::Power(cos(zenith_angle),2.) - TMath::Power(cos(38.*deg_to_rad),2.);

  return ( signal / (1. + a*x + b*TMath::Power(x,2.) + c*TMath::Power(x,3.) ) );
}

void usage()
{
  printf("\nUsage:\n"
  "------ \n\n"
  "This program reads out sequentially the specified ADST files and creates a ROOT file containing \n"
  "ROOT objects that can be vizualized (and compared) by the DQM viewer. The program assumes that \n"
  "the ADSTs to be analyzed are in the standard ADST file structure, namely: \n"
  "\t $PATH_TO_ADST/<Year>/<Month>/Header_<Year>_<Month>_<Day>.root \n\n"
  "Examples:\n"
  "\t To analyze all the data from 2011: ./CreatePeriodFile $PATH_TO_ADST_FILES/2011/*/*.root \n"
  "\t To analyze only the data for the month of May, 2011: ./CreatePeriodFile $PATH_TO_ADST_FILES/2011/05/*.root \n\n"
  "The ROOT files are saved in the current directory as: From_<Year>_<Month>_<Day>_To_<Year>_<Month>_<Day>.root\n\n"
  "NOTE: you can edit this program and add histograms / graphs to be visualized by the DQM viewer. \n"
  "Just make sure that the naming conventions, etc... are respected (and don't forget to re-run the desired periods).\n"
  "\t SD-related objects: pointer names start with sd_ \n"
  "\t FD-related objects: pointer names start with fd<0-5>_ (see code) \n"
  "\t Hybrid-related objects: pointer names start with hy<0-5>_ (see code) \n\n"
  "Objects currently handled by DQM viewer: TH1, TH2, TGraph, TGraphErrors \n\n"
  "Questions / comments? Please contact Fred Sarazin: fsarazin@mines.edu \n\n");
}
