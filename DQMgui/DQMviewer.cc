// C++
#include <iostream> 
#include <fstream>  
#include <sstream> 
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>

// ROOT 
#include <TROOT.h>
#include <TClass.h>
#include <TStyle.h>
#include <TFile.h>
#include <TList.h>
#include <TLine.h>
#include <TH1.h>
#include <TH2.h>
#include <TKey.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TMultiGraph.h>
#include <TApplication.h>
#include <TPad.h>
#include <TF1.h>
#include <TLatex.h>

// ROOT GUI
#include <TGTab.h>
#include <TGProgressBar.h>
#include <TGButton.h>
#include <TGStatusBar.h>
#include <TGComboBox.h>
#include <TCanvas.h>
#include <TRootEmbeddedCanvas.h>
#include <TGTextEntry.h>
#include <TGLabel.h>
#include <TQObject.h>
#include <RQ_OBJECT.h>

// Extra .h file
#include "DQMviewer.h"

using namespace std; // So the compiler interprets cout as std::cout

// Subroutines and functions
void Usage();
int *GetNumberOfTabs(TList *list);
string *GetParsedStrings(string str);
TList *LoadData(int argc, char **argv);
TList *AddAnalyzedObjects(TList *list);
TObject *FindMatchingDqmObject(TObject *ref, TList *list);
TH1 *NormalizeToDQM(TH1 *ref, TH1 *dqm);
TH1 *ApplyHistoStyle(TH1 *h, string header);
TGraphErrors *CreateResidualGraph(TH1 *ref, TH1 *dqm);
TMultiGraph *CreateMultiGraph(TGraph *ref, TGraph *dqm);
TMultiGraph *CreateMultiGraph2(TGraphErrors *ref, TGraphErrors *dqm);
TH1 *CreateConsistencyTest(TH2 *ref, TH2 *dqm);
double meanLDFfunction(double *x, double *par);
double diffmeanLDFfunction(double *x, double *par);

// Global Variables
const int Neyes = 6; // including 0: "all eyes" and 5: "Heat"
const int Nenergyrange = 12;
const int tMax = 100; // max number of tabs
const int kSD = 0, kFD = 1, kHY = 2; // self explanatory
const int sMax = 10; // maxiumum number of parsed strings -- more than enough

string refFile, dqmFile;
int sd_Ntab, fd_Ntab, hy_Ntab;
Pixel_t *sd_status, *fd_status, *hy_status;
TList *obj_list;
TObject *object;

SDGUI::SDGUI(const TGWindow *p, unsigned int w, unsigned int h)
  : TGMainFrame(p, w, h)
{
  int iTabSD, iTabFD, iTabHY, iTabRD, iTabMD;
  string *parsed;
  string header, stream, objectname;
  stringstream ss;

  TGString eyes[Neyes] = {"All eyes","Los Leones","Los Morados","Loma Amarilla","Coihueco","Heat"};
  TGString energyrange[Nenergyrange] = {"All energies","17.8 - 18.0","18.0 - 18.2","18.2 - 18.4",
					"18.4 - 18.6","18.6 - 18.8","18.8 - 19.0","19.0 - 19.2",
					"19.2 - 19.4","19.4 - 19.6","19.6 - 19.8", "19.8 +"};
 
  gStyle->SetOptStat(0);
  
  mainFrame = new TGHorizontalFrame(this, w, h);    
  tab = new TGTab(mainFrame, 1, 1);
  tab->Connect("Selected(Int_t)", "SDGUI", this, "DoTab(Int_t)");

  hint_plots = new TGLayoutHints(kLHintsExpandX|kLHintsExpandY, 2, 2, 2, 2);
  hint_status = new TGLayoutHints(kLHintsExpandX|kLHintsCenterX, 2, 2, 2, 2);
  hint_period = new TGLayoutHints(kLHintsExpandX|kLHintsCenterX, 1, 1, 1, 1);
  hint_seltab = new TGLayoutHints(kLHintsExpandY, 2, 2, 2, 2);
  hint = new TGLayoutHints(kLHintsExpandX|kLHintsExpandY, 2, 2, 2, 2); 

  string ref_period = GetPeriod(refFile);
  string dqm_period = GetPeriod(dqmFile);
  
  // ------------------------
  // SD TAB  
  // ------------------------

  sd_frame = new TGCompositeFrame();
  sd_frame = tab->AddTab("SD");

  sdStatusBar = new TGStatusBar(sd_frame);
  int sdstatusbar_breakdown[] = {15,15,35,35};
  sdStatusBar->SetParts(sdstatusbar_breakdown,4);
  sd_frame->AddFrame(sdStatusBar, new TGLayoutHints(kLHintsExpandX, 1, 1, 1, 1));

  sdStatusBar->AddText("Log(E/eV) range", 0);
  sd_energy = sdStatusBar->GetBarPart(1);
  sdEnergySelect = new TGComboBox(sd_energy,"All energies");
  for (int i = 0; i < Nenergyrange; i++)
    sdEnergySelect->AddEntry(energyrange[i],i);
  sdEnergySelect->Connect("Selected(Int_t)", "SDGUI", this, "EnergySelect(Int_t)");
  sd_energy->AddFrame(sdEnergySelect, new TGLayoutHints(kLHintsExpandX|kLHintsExpandY));

  sd_ref_frame = sdStatusBar->GetBarPart(2);
  sd_dqm_frame = sdStatusBar->GetBarPart(3);
  sd_ref_period = new TGLabel(sd_ref_frame,Form("REF: %s",ref_period.c_str()));
  sd_ref_frame->AddFrame(sd_ref_period, hint_period);
  sd_dqm_period = new TGLabel(sd_dqm_frame,Form("DQM: %s", dqm_period.c_str()));
  sd_dqm_frame->AddFrame(sd_dqm_period, hint_period);  
  
  sd_tab = new TGTab(sd_frame,1,1);
  sd_frame->AddFrame(sd_tab,hint_plots);
  sd_tab->Connect("Selected(Int_t)", "SDGUI", this, "DoTabSD(Int_t)");

  // ------------------------
  // FD TAB
  // ------------------------

  fd_frame = new TGCompositeFrame();
  fd_frame = tab->AddTab("FD");  
  
  fdStatusBar = new TGStatusBar(fd_frame);
  int fdstatusbar_breakdown[] = {15,15,35,35};
  fdStatusBar->SetParts(fdstatusbar_breakdown,4);
  fd_frame->AddFrame(fdStatusBar, new TGLayoutHints(kLHintsExpandX, 1, 1, 1, 1));

  fd_eyebox = fdStatusBar->GetBarPart(1);
  fd_ref_frame = fdStatusBar->GetBarPart(2);
  fd_dqm_frame = fdStatusBar->GetBarPart(3);
  fd_ref_period = new TGLabel(fd_ref_frame,Form("REF: %s",ref_period.c_str()));
  fd_ref_frame->AddFrame(fd_ref_period, hint_period);
  fd_dqm_period = new TGLabel(fd_dqm_frame,Form("DQM: %s", dqm_period.c_str()));
  fd_dqm_frame->AddFrame(fd_dqm_period, hint_period);
  
  fdEyeSelect = new TGComboBox(fd_eyebox,eyes[0]);
  for (int i = 0; i < Neyes; i++)
    fdEyeSelect->AddEntry(eyes[i],i);
  fdEyeSelect->Connect("Selected(Int_t)", "SDGUI", this, "EyeSelect(Int_t)");

  fdStatusBar->AddText("Eye Selection", 0);
  fd_eyebox->AddFrame(fdEyeSelect, new TGLayoutHints(kLHintsExpandX|kLHintsExpandY));
  
  fd_tab = new TGTab(fd_frame,1,1);
  fd_frame->AddFrame(fd_tab, new TGLayoutHints(kLHintsExpandY|kLHintsExpandX));
  fd_tab->Connect("Selected(Int_t)", "SDGUI", this, "DoTabFD(Int_t)");

  // ------------------------
  // HYBRID TAB
  // ------------------------

  hy_frame = new TGCompositeFrame();
  hy_frame = tab->AddTab("Hybrid SD/FD");

  hyStatusBar = new TGStatusBar(hy_frame);
  int hystatusbar_breakdown[] = {15,15,35,35};
  hyStatusBar->SetParts(hystatusbar_breakdown,4);
  hy_frame->AddFrame(hyStatusBar, new TGLayoutHints(kLHintsExpandX, 1, 1, 1, 1));

  hy_eyebox = hyStatusBar->GetBarPart(1);
  hy_ref_frame = hyStatusBar->GetBarPart(2);
  hy_dqm_frame = hyStatusBar->GetBarPart(3);
  hy_ref_period = new TGLabel(hy_ref_frame,Form("REF: %s",ref_period.c_str()));
  hy_ref_frame->AddFrame(hy_ref_period, hint_period);
  hy_dqm_period = new TGLabel(hy_dqm_frame,Form("DQM: %s", dqm_period.c_str()));
  hy_dqm_frame->AddFrame(hy_dqm_period, hint_period);

  hyEyeSelect = new TGComboBox(hy_eyebox,eyes[0]);
  for (int i = 0; i < Neyes; i++)
    hyEyeSelect->AddEntry(eyes[i],i);
  hyEyeSelect->Connect("Selected(Int_t)", "SDGUI", this, "EyeSelect(Int_t)");

  hyStatusBar->AddText("Eye Selection", 0);
  hy_eyebox->AddFrame(hyEyeSelect, new TGLayoutHints(kLHintsExpandX|kLHintsExpandY));

  hy_tab = new TGTab(hy_frame,1,1);
  hy_frame->AddFrame(hy_tab,hint_plots);
  hy_tab->Connect("Selected(Int_t)", "SDGUI", this, "DoTabHY(Int_t)");

  // ------------------------
  // RADIO TAB
  // ------------------------

  rd_frame = new TGCompositeFrame();
  rd_frame = tab->AddTab("RD");

  rdStatusBar = new TGStatusBar(rd_frame);
  int rdstatusbar_breakdown[] = {50,50};
  rdStatusBar->SetParts(rdstatusbar_breakdown,2);
  rd_frame->AddFrame(rdStatusBar, new TGLayoutHints(kLHintsExpandX, 1, 1, 1, 1));

  rd_ref_frame = rdStatusBar->GetBarPart(0);
  rd_dqm_frame = rdStatusBar->GetBarPart(1);
  rd_ref_period = new TGLabel(rd_ref_frame,Form("REF: %s",ref_period.c_str()));
  rd_ref_frame->AddFrame(rd_ref_period, hint_period);
  rd_dqm_period = new TGLabel(rd_dqm_frame,Form("DQM: %s", dqm_period.c_str()));
  rd_dqm_frame->AddFrame(rd_dqm_period, hint_period);

  rd_tab = new TGTab(rd_frame,1,1);
  rd_frame->AddFrame(rd_tab,hint_plots);
  rd_tab->Connect("Selected(Int_t)", "SDGUI", this, "DoTabRD(Int_t)");

  // ------------------------
  // MUON TAB
  // ------------------------

  md_frame = new TGCompositeFrame();
  md_frame = tab->AddTab("MD");

  mdStatusBar = new TGStatusBar(md_frame);
  int mdstatusbar_breakdown[] = {50,50};
  mdStatusBar->SetParts(mdstatusbar_breakdown,2);
  md_frame->AddFrame(mdStatusBar, new TGLayoutHints(kLHintsExpandX, 1, 1, 1, 1));

  md_ref_frame = mdStatusBar->GetBarPart(0);
  md_dqm_frame = mdStatusBar->GetBarPart(1);
  md_ref_period = new TGLabel(md_ref_frame,Form("REF: %s",ref_period.c_str()));
  md_ref_frame->AddFrame(md_ref_period, hint_period);
  md_dqm_period = new TGLabel(md_dqm_frame,Form("DQM: %s", dqm_period.c_str()));
  md_dqm_frame->AddFrame(md_dqm_period, hint_period);

  md_tab = new TGTab(md_frame,1,1);
  md_frame->AddFrame(md_tab,hint_plots);
  md_tab->Connect("Selected(Int_t)", "SDGUI", this, "DoTabMD(Int_t)");

  // ------------------------
  // ADDING ALL THE OBJECTS
  // ------------------------

  TIter next(obj_list);
  iTabSD = 0;
  iTabFD = 0;
  iTabHY = 0;
  iTabRD = 0;
  iTabMD = 0;
  while ( (object = next()) ) {    
    parsed = GetParsedStrings(object->GetName());
    header = parsed[0];
    stream = parsed[1];
    objectname = parsed[2];
    if ( ( (header == "ref") && ((string)object->ClassName() != "TGraph") 
	                     && ((string)object->ClassName() != "TGraphErrors") ) || // TGraphs are handled with TMultiGraph
	   (header == "mgr") ) {     
      if (stream == "sd0") {
	sd_objtabs[iTabSD] = sd_tab->AddTab(objectname.c_str());
	sd_canvas[iTabSD] = new TRootEmbeddedCanvas(objectname.c_str(), sd_objtabs[iTabSD], w, h);
	sd_objtabs[iTabSD]->AddFrame(sd_canvas[iTabSD],hint);
	iTabSD++;
      }
      if (stream == "fd0") {
	fd_objtabs[iTabFD] = fd_tab->AddTab(objectname.c_str());
	fd_canvas[iTabFD] = new TRootEmbeddedCanvas(objectname.c_str(), fd_objtabs[iTabFD], w, h);
	fd_objtabs[iTabFD]->AddFrame(fd_canvas[iTabFD],hint);
	iTabFD++;
      }
      if (stream == "hy0") {
	hy_objtabs[iTabHY] = hy_tab->AddTab(objectname.c_str());
	hy_canvas[iTabHY] = new TRootEmbeddedCanvas(objectname.c_str(), hy_objtabs[iTabHY], w, h);
	hy_objtabs[iTabHY]->AddFrame(hy_canvas[iTabHY],hint);
	iTabHY++;
      }
      if (stream == "rd") {
	rd_objtabs[iTabRD] = rd_tab->AddTab(objectname.c_str());
	rd_canvas[iTabRD] = new TRootEmbeddedCanvas(objectname.c_str(), rd_objtabs[iTabRD], w, h);
	rd_objtabs[iTabRD]->AddFrame(rd_canvas[iTabRD],hint);
	iTabRD++;
      }
      if (stream == "md") {
	md_objtabs[iTabMD] = md_tab->AddTab(objectname.c_str());
	md_canvas[iTabMD] = new TRootEmbeddedCanvas(objectname.c_str(), md_objtabs[iTabMD], w, h);
	md_objtabs[iTabMD]->AddFrame(md_canvas[iTabMD],hint);
	iTabMD++;
      }
    }
  }

  mainFrame->AddFrame(tab, hint);
  
  AddFrame(mainFrame, hint);
  
  // ------------------------
  // STATUS BAR
  // ------------------------

  fileStatusBar = new TGStatusBar(this);
  int filestatusbar_breakdown[] = {50,50};
  fileStatusBar->SetParts(filestatusbar_breakdown,2);
  ss << "REF: " << refFile;
  fileStatusBar->SetText(ss.str().c_str(),0);
  ss.str("");
  ss << "DQM: " << dqmFile;
  fileStatusBar->SetText(ss.str().c_str(),1);
  ss.str("");
  AddFrame(fileStatusBar, hint_status);
  
  tab->SetTab(0);
  MapSubwindows();
  Resize(GetDefaultSize());
  SetWindowName("Data Quality Monitoring (DQM) graphical user interface");
  MapWindow();

  sd_status = DrawRecPlots("sd0",sd_canvas);
  fd_status = DrawRecPlots("fd0",fd_canvas);
  hy_status = DrawRecPlots("hy0",hy_canvas);
 
  UpdateTabColorStatus(sd_status,sd_tab,sd_objtabs);
  UpdateTabColorStatus(fd_status,fd_tab,fd_objtabs);
  UpdateTabColorStatus(hy_status,hy_tab,hy_objtabs);
}

SDGUI::~SDGUI(){
  Cleanup();

  for (int i=0;i<sd_Ntab;i++)
    delete sd_canvas[i];
  for (int i=0;i<fd_Ntab;i++)
    delete fd_canvas[i];
  for (int i=0;i<hy_Ntab;i++)
    delete hy_canvas[i];
}

string SDGUI::GetPeriod(string filename)
{
  string *parsed;
  stringstream ss;
  parsed = GetParsedStrings(filename);
  string YearStart = parsed[1];
  string MonthStart = parsed[2];
  string DayStart = parsed[3];
  string YearStop = parsed[5];
  string MonthStop = parsed[6];
  string DayStop = parsed[7].substr(0,2);

  ss << "From " << YearStart << "/" << MonthStart << "/" << DayStart 
     << " to " << YearStop << "/" << MonthStop << "/" << DayStop;

  return ss.str();
}

string *SDGUI::GetParsedStrings(string str)
{
  int iString = 0;

  stringstream stream(str);
  string parsed_string;
  string* parsed = new string[sMax];
  while (getline(stream,parsed_string,'_')) {
    parsed[iString] = parsed_string; 
    iString++;
  }

  return parsed;
}

TObject *SDGUI::FindMatchingDqmObject(TObject *ref, TList *list)
{
  TObject *dqm;
  string *parsed;
  string header, stream, objectname;
  stringstream ss;
    
  parsed = GetParsedStrings(ref->GetName());
  header = parsed[0];
  stream = parsed[1];
  objectname = parsed[2];
 
  ss << "dqm_" << stream << "_" << objectname;

  dqm = (TObject*)list->FindObject(ss.str().c_str());
  
  return dqm;
}

TGraphErrors *SDGUI::FindMatchingResidual(TH1 *ref, TList *list)
{
  TGraphErrors *res;
  
  string *parsed;
  string header, stream, histoname;
  stringstream ss;
 
  parsed = GetParsedStrings(ref->GetName());
  header = parsed[0];
  stream = parsed[1];
  histoname = parsed[2];
 
  ss << "res_" << stream << "_" << histoname;

  res = (TGraphErrors*)list->FindObject(ss.str().c_str());
  
  return res;
}

TH1 *SDGUI::FindMatchingConsistencyTest(TH2 *ref, TList *list)
{
  TH1 *ctest;
  
  string *parsed;
  string header, stream, histoname;
  stringstream ss;
 
  parsed = GetParsedStrings(ref->GetName());
  header = parsed[0];
  stream = parsed[1];
  histoname = parsed[2];
 
  ss << "con_" << stream << "_" << histoname;

  ctest = (TH1*)list->FindObject(ss.str().c_str());

  return ctest;
}

TGraph *SDGUI::GetGraphFromMultiGraph(TMultiGraph *mg, string header)
{
  TGraph *graph;
  string *parsed;
  string stream, graphname;
  stringstream ss;
  parsed = GetParsedStrings(mg->GetName());
  stream = parsed[1];
  graphname = parsed[2];

  ss << header << "_" << stream << "_" << graphname;

  graph = (TGraph*)mg->GetListOfGraphs()->FindObject(ss.str().c_str());

  graph = ApplyGraphStyle(graph,header);

  return graph;
}

Pixel_t *SDGUI::DrawRecPlots(string selected_stream, TRootEmbeddedCanvas **canvas) 
{
  TObject *obj;
  
  string *parsed;
  string header, stream;
  stringstream ss;

  Pixel_t *rec_status = (Pixel_t*)malloc( tMax * sizeof(Pixel_t) );
  
  int iTab = 0;

  if (selected_stream.substr(0,2) == "sd") canvas = sd_canvas;
  if (selected_stream.substr(0,2) == "fd") canvas = fd_canvas;
  if (selected_stream.substr(0,2) == "hy") canvas = hy_canvas;

  TIter next(obj_list);
  while ( (obj = next()) ) {
    parsed = GetParsedStrings(obj->GetName());
    header = parsed[0];
    stream = parsed[1];

    if ( (header == "ref") && (stream == selected_stream) ) {
      if ( ((string)obj->ClassName() == "TH1F") || ((string)obj->ClassName() == "TH1D") ) {
	canvas[iTab]->GetCanvas()->Clear();
	canvas[iTab]->GetCanvas()->cd();
	rec_status[iTab] = DrawHistogram1D((TH1*)obj,canvas[iTab]);
	iTab++;
      }
      if ( ((string)obj->ClassName() == "TH2F") || ((string)obj->ClassName() == "TH2D") ) {
	canvas[iTab]->GetCanvas()->Clear();
	canvas[iTab]->GetCanvas()->cd();
	rec_status[iTab] = DrawHistogram2D((TH2*)obj,canvas[iTab]);
	iTab++;
      }
    }
    if ( (header == "mgr") && (stream == selected_stream) ) {
      canvas[iTab]->GetCanvas()->Clear();
      canvas[iTab]->GetCanvas()->cd();
      rec_status[iTab] = DrawMultiGraph((TMultiGraph*)obj,canvas[iTab]);
      iTab++;
    }
  }

  return rec_status;
}

Pixel_t SDGUI::DrawHistogram1D(TH1* histo1d_ref, TRootEmbeddedCanvas *canvas)
{
  string *parsed;
  string header, stream, histoname;
  stringstream ss;
  Pixel_t status;

  TH1 *histo1d_dqm;
  TGraphErrors *residual;
  TF1 *function;

  TLatex *text_nodata_1 = new TLatex(0.5,0.6,"NO DATA TO COMPARE!");
  text_nodata_1->SetTextSize(0.10);
  text_nodata_1->SetTextAlign(22);
  text_nodata_1->SetNDC(kTRUE);
  TLatex *text_nodata_2 = new TLatex(0.5,0.4,"At least one histogram empty");
  text_nodata_2->SetTextSize(0.08);
  text_nodata_2->SetTextAlign(22);
  text_nodata_2->SetNDC(kTRUE);

  histo1d_dqm = (TH1*)FindMatchingDqmObject(histo1d_ref,obj_list);

  histo1d_ref = ApplyHistogramStyle(histo1d_ref);
  histo1d_dqm = ApplyHistogramStyle(histo1d_dqm);

  parsed = GetParsedStrings(histo1d_ref->GetName());
  histoname = parsed[2];

  canvas->GetCanvas()->Clear();
  canvas->GetCanvas()->cd();

  ss << "cpad_1_" << histoname;
  TPad *c_pad1 = new TPad(ss.str().c_str(),"Data",0,0.40,1,1);
  ss.str(""); // will reuse
  c_pad1->SetMargin(0.075,0.025,0.1,0.1);
  c_pad1->Draw();
  c_pad1->cd();
  if ( (histoname == "LogS1000") || (histoname == "LogE") || (histoname == "Multiplicity") || 
       (histoname == "SdFdTime") )
    gPad->SetLogy(1);
  else
    gPad->SetLogy(0);
  
  histo1d_dqm->Draw("E"); // to get the full scale of DQM plot.
  histo1d_ref->Draw("E3same");
  histo1d_dqm->Draw("Esame");
  
  canvas->GetCanvas()->cd();
  
  ss << "cpad_2_" << histoname;
  TPad *c_pad2 = new TPad(ss.str().c_str(),"Data",0,0,0.75,0.40);
  ss.str("");
  c_pad2->SetMargin(0.1,0.025,0.15,0.05);
  c_pad2->Draw();
  c_pad2->cd();
  residual = FindMatchingResidual(histo1d_ref,obj_list); 
  residual = ApplyResidualStyle(residual,histo1d_ref);
  residual->Draw("AP");

  canvas->GetCanvas()->cd();      
  ss << "cpad_3_" << histoname;
  TPad *c_pad3 = new TPad(ss.str().c_str(),"Data",0.75,0,1,0.40);
  ss.str("");
  c_pad3->Draw();
  c_pad3->cd();

  if ( (histo1d_ref->Integral() > 0) && (histo1d_dqm->Integral() > 0) ) {
    residual->Fit("pol0","Q");
    function = residual->GetFunction("pol0");
    DisplayHistogramStat(function);
    status = GetHistogramStatus(function);
  }
  else {
    canvas->GetCanvas()->cd();
    c_pad1->cd();
    text_nodata_1->Draw("same");
    text_nodata_2->Draw("same");
    status = 0xEEEEEE;// GUI grey (Gallery)
  }

  canvas->GetCanvas()->Modified();
  canvas->GetCanvas()->Update();
  
  return status;
}

Pixel_t SDGUI::DrawHistogram2D(TH2* histo2d_ref, TRootEmbeddedCanvas *canvas)
{
  string *parsed;
  string histoname;
  stringstream ss;
  string option;
  Pixel_t status;

  TH1 *consistency_test;
  TH2 *histo2d_dqm;  

  TLatex *text_nodata_1 = new TLatex(0.5,0.6,"NO DATA TO SHOW!");
  text_nodata_1->SetTextSize(0.10);
  text_nodata_1->SetTextAlign(22);
  text_nodata_1->SetNDC(kTRUE);
  TLatex *text_nodata_2 = new TLatex(0.5,0.4,"Histogram empty");
  text_nodata_2->SetTextSize(0.08);
  text_nodata_2->SetTextAlign(22);
  text_nodata_2->SetNDC(kTRUE);

  histo2d_dqm = (TH2*)FindMatchingDqmObject(histo2d_ref,obj_list);

  parsed = GetParsedStrings(histo2d_ref->GetName());
  histoname = parsed[2];

  if (histoname == "Skymap")
    option = "aitoff";
  else
    option = "col";

  canvas->GetCanvas()->Clear();
  canvas->GetCanvas()->cd();
	
  ss << "cpad_1_" << histoname;
  TPad *c_pad1 = new TPad(ss.str().c_str(),"Data",0,0.5,0.5,1);
  ss.str("");
  c_pad1->SetMargin(0.075,0.025,0.1,0.1);
  c_pad1->Draw();
  c_pad1->cd();
  c_pad1->SetLogz(1);
  histo2d_ref->Draw(option.c_str());
  ss << "REF: " << histo2d_ref->GetTitle();
  histo2d_ref->SetTitle(ss.str().c_str());
  ss.str("");
  if (histo2d_ref->Integral()==0) {
    text_nodata_1->Draw("same");
    text_nodata_2->Draw("same");
  }

  canvas->GetCanvas()->cd();
  
  ss << "cpad_2_" << histoname;
  TPad *c_pad2 = new TPad(ss.str().c_str(),"Data",0,0,0.5,0.5);
  ss.str("");
  c_pad2->SetMargin(0.075,0.025,0.1,0.1);
  c_pad2->Draw();
  c_pad2->cd();
  c_pad2->SetLogz(1);
  histo2d_dqm->Draw(option.c_str());
  ss << "DQM: " << histo2d_dqm->GetTitle();
  histo2d_dqm->SetTitle(ss.str().c_str());
  ss.str("");
  if (histo2d_dqm->Integral()==0) {
    text_nodata_1->Draw("same");
    text_nodata_2->Draw("same");
  }

  canvas->GetCanvas()->cd();
  
  ss << "cpad_3_" << histoname;
  TPad *c_pad3 = new TPad(ss.str().c_str(),"Data",0.5,0,1,1);
  ss.str("");
  c_pad3->Draw();
  c_pad3->cd();
  gPad->SetLogy(1);

  consistency_test = (TH1*)FindMatchingConsistencyTest(histo2d_ref,obj_list);  
  consistency_test->Draw();

  canvas->GetCanvas()->Modified();
  canvas->GetCanvas()->Update();
  
  status = 0xEEEEEE;// GUI grey (Gallery)

  return status;
}

Pixel_t SDGUI::DrawMultiGraph(TMultiGraph *mg, TRootEmbeddedCanvas *canvas) 
{
  string *parsed;
  string stream;
  string multigraphname;
  stringstream ss;

  Pixel_t status;

  TGraph *gREF, *gDQM;
  TF1 *fREF, *fDQM, *fDIFF;

  double rCoreMin;
  double rCoreMax;

  TLatex *text_nodata_1 = new TLatex(0.5,0.6,"NO DATA TO COMPARE!");
  text_nodata_1->SetTextSize(0.10);
  text_nodata_1->SetTextAlign(22);
  text_nodata_1->SetNDC(kTRUE);
  TLatex *text_nodata_2 = new TLatex(0.5,0.4,"At least one graph empty");
  text_nodata_2->SetTextSize(0.08);
  text_nodata_2->SetTextAlign(22);
  text_nodata_2->SetNDC(kTRUE);

  parsed = GetParsedStrings(mg->GetName());
  stream = parsed[1];
  multigraphname = parsed[2];

  gREF = GetGraphFromMultiGraph(mg,"ref");
  gDQM = GetGraphFromMultiGraph(mg,"dqm");

  canvas->GetCanvas()->Clear();
  canvas->GetCanvas()->cd();

  ss << "cpad_1_" << multigraphname;
  TPad *c_pad1 = new TPad(ss.str().c_str(),"Data",0.,0.40,0.70,1.);
  ss.str(""); // will reuse
  c_pad1->SetMargin(0.075,0.025,0.1,0.1);
  /*
  TPad *c_pad1 = new TPad(ss.str().c_str(),"Data",0,0,0.75,1);
  ss.str("");
  c_pad1->SetMargin(0.075,0.025,0.1,0.1);
  */
  c_pad1->Draw();
  c_pad1->cd();

  if (multigraphname=="MeanLDF")
    gPad->SetLogy(1);


  mg->Draw("AP");
  mg = ApplyMultiGraphStyle(mg);

  if ( (gREF->GetN()==0) || (gDQM->GetN()==0) ) {
    text_nodata_1->Draw();
    text_nodata_2->Draw();
  }

  canvas->GetCanvas()->cd();

  ss << "cpad_2_" << multigraphname;
  TPad *c_pad2 = new TPad(ss.str().c_str(),"Data",0,0,0.70,0.40);
  ss.str("");
  c_pad2->SetMargin(0.075,0.025,0.1,0.1);
  //c_pad2->SetMargin(0.1,0.025,0.15,0.05);
  c_pad2->Draw();
  c_pad2->cd();

  if (multigraphname=="MeanLDF" && stream!="sd0") {
    fREF = new TF1("fREF",meanLDFfunction,gREF->GetXaxis()->GetXmin(),gREF->GetXaxis()->GetXmax(),3);
    fREF->SetLineWidth(2);
    fREF->SetLineColor(kRed-10);
    fREF->SetParameter(1,-2.); // beta initial guess
    fREF->SetParameter(2,0.); // gamma initial guess
    fREF->SetParLimits(2,0.,5.);
    fDQM = new TF1("fDQM",meanLDFfunction,gDQM->GetXaxis()->GetXmin(),gDQM->GetXaxis()->GetXmax(),3);
    fDQM->SetLineWidth(2);
    fDQM->SetLineColor(kRed);
    fDQM->SetParameter(1,-2.); // beta initial guess
    fDQM->SetParameter(2,0.); // gamma initial guess
    fDQM->SetParLimits(2,0.,5.);

    gREF->Fit("fREF","Q");
    gDQM->Fit("fDQM","Q");

    rCoreMin = fREF->GetX(500.); // when S = 500 
    rCoreMax = fREF->GetX(10.); // when S = 10

    c_pad1->cd();
    TLine *minrCoreDist_pad1 = new TLine(rCoreMin,mg->GetYaxis()->GetXmin(),rCoreMin,mg->GetYaxis()->GetXmax());
    minrCoreDist_pad1->SetLineStyle(2);
    minrCoreDist_pad1->SetLineColor(kBlack);
    TLine *maxrCoreDist_pad1 = new TLine(rCoreMax,mg->GetYaxis()->GetXmin(),rCoreMax,mg->GetYaxis()->GetXmax());
    maxrCoreDist_pad1->SetLineStyle(2);
    maxrCoreDist_pad1->SetLineColor(kBlack);
    minrCoreDist_pad1->Draw();
    maxrCoreDist_pad1->Draw();

    c_pad2->cd();
    fDIFF = new TF1("fDIFF",diffmeanLDFfunction,rCoreMin-100.,rCoreMax+100.,6.);
    fDIFF->SetTitle("Residual: (Ref-Dqm) / Ref");
    fDIFF->SetLineColor(kRed);
    fDIFF->SetLineWidth(2);
    fDIFF->FixParameter(0,fREF->GetParameter(0)); // S38(ref)
    fDIFF->FixParameter(1,fREF->GetParameter(1)); // Beta(ref)
    fDIFF->FixParameter(2,fREF->GetParameter(2)); // Gamma(ref)
    fDIFF->FixParameter(3,fDQM->GetParameter(0)); // S38(dqm)
    fDIFF->FixParameter(4,fDQM->GetParameter(1)); // Beta(dqm)
    fDIFF->FixParameter(5,fDQM->GetParameter(2)); // Gamma(dqm)
    fDIFF->GetXaxis()->SetTitle(mg->GetXaxis()->GetTitle());
    fDIFF->GetYaxis()->SetTitle(mg->GetYaxis()->GetTitle());
    fDIFF->GetYaxis()->SetRangeUser(-0.5,0.5);
    fDIFF->Draw();
    gPad->Modified();

    //TLine *minrCoreDist_pad2 = new TLine(rCoreMin,gPad->GetUymin(),rCoreMin,gPad->GetUymax());
    TLine *minrCoreDist_pad2 = new TLine(rCoreMin,-0.5,rCoreMin,0.5);
    minrCoreDist_pad2->SetLineStyle(2);
    minrCoreDist_pad2->SetLineColor(kBlack);
    //TLine *maxrCoreDist_pad2 = new TLine(rCoreMax,gPad->GetUymin(),rCoreMax,gPad->GetUymax());
    TLine *maxrCoreDist_pad2 = new TLine(rCoreMax,-0.5,rCoreMax,0.5);
    maxrCoreDist_pad2->SetLineStyle(2);
    maxrCoreDist_pad2->SetLineColor(kBlack);
    TLine *zero_pad2 = new TLine(rCoreMin-100.,0.,rCoreMax+100.,0.);
    zero_pad2->SetLineStyle(2);
    zero_pad2->SetLineColor(kBlack);
    TLine *greenplus_pad2 = new TLine(rCoreMin-100.,0.1,rCoreMax+100.,0.1);
    greenplus_pad2->SetLineStyle(2);
    greenplus_pad2->SetLineColor(kGreen+1);
    TLine *greenminus_pad2 = new TLine(rCoreMin-100.,-0.1,rCoreMax+100.,-0.1);
    greenminus_pad2->SetLineStyle(2);
    greenminus_pad2->SetLineColor(kGreen+1);
    TLine *orangeplus_pad2 = new TLine(rCoreMin-100.,0.25,rCoreMax+100.,0.25);
    orangeplus_pad2->SetLineStyle(2);
    orangeplus_pad2->SetLineColor(kOrange+1);
    TLine *orangeminus_pad2 = new TLine(rCoreMin-100.,-0.25,rCoreMax+100.,-0.25);
    orangeminus_pad2->SetLineStyle(2);
    orangeminus_pad2->SetLineColor(kOrange+1);

    minrCoreDist_pad2->Draw();
    maxrCoreDist_pad2->Draw();
    zero_pad2->Draw();
    greenplus_pad2->Draw();
    greenminus_pad2->Draw();
    orangeplus_pad2->Draw();
    orangeminus_pad2->Draw();

    canvas->GetCanvas()->cd();
    ss << "cpad_3_" << multigraphname;
    TPad *c_pad3 = new TPad(ss.str().c_str(),"Data",0.70,0.,1,1);
    ss.str("");
    c_pad3->Draw();
    c_pad3->cd();
  
    DisplayLDFMultiGraphStat(gREF,gDQM,fREF,fDQM);
    status = GetLDFMultiGraphStatus(fDIFF,rCoreMin,rCoreMax);
  }
  else
    status = 0xEEEEEE;

  canvas->GetCanvas()->Modified();
  canvas->GetCanvas()->Update();

  return status;
}

double meanLDFfunction(double *x, double *par) 
{
  // From A.Herve - GAP-2013-076 (p.22)
  double xx = x[0];
  double S38 = par[0];
  double beta = par[1];
  double gamma = par[2];
  double extraterm = (1.+TMath::Power(xx/3000.,3.)) / (1.+TMath::Power(1000./3000.,3.));
  double f = S38 * TMath::Power(xx/1000.,beta) * TMath::Power((xx+700.)/(1000.+700.),beta) * TMath::Power(extraterm,gamma);
  return f;
}

double diffmeanLDFfunction(double *x, double *par) 
{
  double xx = x[0];
  double S38_ref = par[0];
  double beta_ref = par[1];
  double gamma_ref = par[2];
  double S38_dqm = par[3];
  double beta_dqm = par[4];
  double gamma_dqm = par[5];
  double extraterm = (1.+TMath::Power(xx/3000.,3.)) / (1.+TMath::Power(1000./3000.,3.));

  double f_ref = S38_ref * TMath::Power(xx/1000.,beta_ref) * TMath::Power((xx+700.)/(1000.+700.),beta_ref) * TMath::Power(extraterm,gamma_ref);
  double f_dqm = S38_dqm * TMath::Power(xx/1000.,beta_dqm) * TMath::Power((xx+700.)/(1000.+700.),beta_dqm) * TMath::Power(extraterm,gamma_dqm);

  return (f_ref-f_dqm)/f_ref;
}

TH1 *SDGUI::ApplyHistogramStyle(TH1 *h)
{
  string *parsed;
  string header, stream, histoname;

  int color_ref[6] = {kGray,kBlue-10,kMagenta-10,kRed-10,kGreen-10,kYellow-10};
  int color_dqm[6] = {kBlack,kBlue+1,kMagenta+1,kRed+1,kGreen+1,kYellow+1};

  int color_picked = 0;

  parsed = GetParsedStrings(h->GetName());
  header = parsed[0];
  stream = parsed[1];
  histoname = parsed[2];

  if ( (stream.substr(0,2)=="fd") || (stream.substr(0,2)=="hy") ){
    for (int i=0;i<Neyes;i++)
      if (stoi(stream.substr(2,1))==i) 
	color_picked = i;
  }

  if ( ((string)h->ClassName() == "TH1F") || ( (string)h->ClassName() == "TH1D") ) {
    if (header == "ref")
      h->SetFillColor(color_ref[color_picked]);
    else { // dqm
      h->SetLineColor(color_dqm[color_picked]);
      h->SetLineWidth(2);
    }
  }

  return h;
}

TGraph *SDGUI::ApplyGraphStyle(TGraph *g, string header)
{
  if ( ((string)g->ClassName() == "TGraph") || ((string)g->ClassName() == "TGraphErrors") ) {
    g->SetMarkerStyle(20);
    g->SetMarkerSize(0.75);
    if (header == "ref") {
      g->SetMarkerColor(kGray);
      g->SetLineColor(kGray);
    }
    else { // dqm
      g->SetMarkerColor(kBlack);
      g->SetLineColor(kBlack);
    }
  }
  
  return g;
}

TGraphErrors *SDGUI::ApplyResidualStyle(TGraphErrors *ge, TH1 *histo1d_ref)
{
  string *parsed;
  string header, stream, histoname;

  int color[6] = {kBlack,kBlue+1,kMagenta+1,kRed+1,kGreen+1,kYellow+1};

  int color_picked = 0;

  parsed = GetParsedStrings(ge->GetName());
  header = parsed[0];
  stream = parsed[1];
  histoname = parsed[2];

  if ( (stream.substr(0,2)=="fd") || (stream.substr(0,2)=="hy") ){
    for (int i=0;i<Neyes;i++)
      if (stoi(stream.substr(2,1))==i) 
	color_picked = i;
  }

  if ( ((string)ge->ClassName() == "TGraph") || ( (string)ge->ClassName() == "TGraphErrors") ) {
    ge->SetMarkerColor(color[color_picked]);
    ge->SetLineColor(color[color_picked]);
  }

  return ge;
}

TMultiGraph *SDGUI::ApplyMultiGraphStyle(TMultiGraph *mg)
{
  TGraph *gREF = GetGraphFromMultiGraph(mg,"ref");
  mg->GetXaxis()->SetTitle(gREF->GetXaxis()->GetTitle());
  mg->GetYaxis()->SetTitle(gREF->GetYaxis()->GetTitle());
  mg->GetXaxis()->SetRangeUser(gREF->GetXaxis()->GetXmin(),gREF->GetXaxis()->GetXmax());
  mg->GetXaxis()->SetLabelSize(0.025);
  mg->GetYaxis()->SetLabelSize(0.025);

  return mg;
}

void SDGUI::DisplayHistogramStat(TF1 *func) {
  gStyle->SetTextSize(0.1);
  char compositeText[128];
  sprintf(compositeText,"\t #mu: %4.3f",func->GetParameter(0));
  TLatex *t_mean = new TLatex(0.,0.85,compositeText);
  t_mean->Draw();
  sprintf(compositeText,"\t #sigma: %4.3f",func->GetParError(0));
  TLatex *t_meanerr = new TLatex(0.,0.70,compositeText);
  t_meanerr->Draw();
  sprintf(compositeText,"\t #chi^{2}: %4.3f",func->GetChisquare());
  TLatex *t_chisq = new TLatex(0.,0.55,compositeText);
  t_chisq->Draw();
  sprintf(compositeText,"\t NDoF: %d",func->GetNDF());
  TLatex *t_ndof = new TLatex(0.,0.40,compositeText);
  t_ndof->Draw();
  sprintf(compositeText,"\t #chi^{2}/NDoF: %4.3f",func->GetChisquare()/func->GetNDF());
  TLatex *t_redchisq = new TLatex(0.,0.25,compositeText);
  t_redchisq->Draw();
}

void SDGUI::DisplayLDFMultiGraphStat(TGraph *gREF, TGraph *gDQM, TF1 *fREF, TF1 *fDQM) 
{  
  char compositeText[128];

  gStyle->SetTextSize(0.075);
  TLatex *t_refheader = new TLatex(0.,0.85,"Mean LDF (REF):");
  t_refheader->Draw();
  sprintf(compositeText,"\t S38_{Ref}: %4.2f #pm %4.2f",fREF->GetParameter(0),fREF->GetParError(0));
  TLatex *t_refS38 = new TLatex(0.,0.75,compositeText);
  t_refS38->Draw();
  sprintf(compositeText,"\t #beta_{Ref}: %4.2f #pm %4.2f",fREF->GetParameter(1),fREF->GetParError(1));
  TLatex *t_refBeta = new TLatex(0.,0.65,compositeText);
  t_refBeta->Draw();
  sprintf(compositeText,"\t #gamma_{Ref}: %4.2f #pm %4.2f",fREF->GetParameter(2),fREF->GetParError(2));
  TLatex *t_refGamma = new TLatex(0.,0.55,compositeText);
  t_refGamma->Draw();

  TLatex *t_dqmheader = new TLatex(0.,0.45,"Mean LDF (DQM):");
  t_dqmheader->Draw();
  sprintf(compositeText,"\t S38_{Dqm}: %4.2f #pm %4.2f",fDQM->GetParameter(0),fDQM->GetParError(0));
  TLatex *t_dqmS38 = new TLatex(0.,0.35,compositeText);
  t_dqmS38->Draw();
  sprintf(compositeText,"\t #beta_{Dqm}: %4.2f #pm %4.2f",fDQM->GetParameter(1),fDQM->GetParError(1));
  TLatex *t_dqmBeta = new TLatex(0.,0.25,compositeText);
  t_dqmBeta->Draw();
  sprintf(compositeText,"\t #gamma_{Dqm}: %4.2f #pm %4.2f",fDQM->GetParameter(2),fDQM->GetParError(2));
  TLatex *t_dqmGamma = new TLatex(0.,0.15,compositeText);
  t_dqmGamma->Draw();
}

Pixel_t SDGUI::GetHistogramStatus(TF1 *func)
{
  Pixel_t color;
  double Val = func->GetParameter(0);
  double eVal = func->GetParError(0);
  
  double chiSq = func->GetChisquare();
  int NDF = func->GetNDF();
  double rchiSq = chiSq/double(NDF);
  
  // Based on Reduced Chi Sq.
  if (rchiSq<1.25)
    color = 0x99FF99; // light green
  else if (rchiSq<2.5)
    color = 0xFFCC66; // light orange
  else 
    color = 0xFF9999; // light red 
  
  return color;
}

Pixel_t SDGUI::GetLDFMultiGraphStatus(TF1 *f, double min, double max)
{
  Pixel_t color;
  TF1 *func = (TF1*)f->Clone();
  double Ymin = func->GetMinimum();
  double Ymax = func->GetMaximum();

  if ( (Ymin<-0.25) || (Ymax>0.25) ) 
    color = 0xFF9999; // light red 
  else
    if ( (Ymin<-0.10) || (Ymax>0.10) ) color = 0xFFCC66; // light orange
    else
      color = 0x99FF99; // light green
  
  return color;
}

void SDGUI::UpdateTabColorStatus(Pixel_t *status, TGTab *tab, TGCompositeFrame **htab)
{
  TGTabElement *tlabel;
  int curr = tab->GetCurrent();
  int Number_of_Tabs = tab->GetNumberOfTabs();
  for (int itab = 0; itab<Number_of_Tabs; itab++) {
    htab[itab]->SetBackgroundColor(status[itab]);
    tlabel = tab->GetTabTab(itab);
    tab->SetTab(itab);
    tlabel->SetBackgroundColor(status[itab]);
  }
  tab->SetTab(curr);
}

void SDGUI::ClearList(){
  obj_list->Clear();
}

void SDGUI::DoTab(Int_t id){
  int nTab;
  if (id==0) { // SD tab
    nTab = sd_tab->GetCurrent();
    sd_canvas[nTab]->GetCanvas()->Resize();
    sd_canvas[nTab]->GetCanvas()->Modified();
    sd_canvas[nTab]->GetCanvas()->Update();    
  }
  if (id==1) { // FD tab    
    nTab = fd_tab->GetCurrent();
    fd_canvas[nTab]->GetCanvas()->Resize();
    fd_canvas[nTab]->GetCanvas()->Modified();
    fd_canvas[nTab]->GetCanvas()->Update();
  }
  if (id==2) { // HY tab
    nTab = hy_tab->GetCurrent();
    hy_canvas[nTab]->GetCanvas()->Resize();
    hy_canvas[nTab]->GetCanvas()->Modified();
    hy_canvas[nTab]->GetCanvas()->Update();
  }  
}

void SDGUI::UpdateTab(int id, TRootEmbeddedCanvas **canvas)
{  
  canvas[id]->GetCanvas()->Resize();
  canvas[id]->GetCanvas()->Modified();
  canvas[id]->GetCanvas()->Update();
}

void SDGUI::DoTabSD(Int_t id){  
  sd_canvas[id]->GetCanvas()->Resize();
  sd_canvas[id]->GetCanvas()->Modified();
  sd_canvas[id]->GetCanvas()->Update();
}

void SDGUI::DoTabFD(Int_t id){
  fd_canvas[id]->GetCanvas()->Resize();
  fd_canvas[id]->GetCanvas()->Modified();
  fd_canvas[id]->GetCanvas()->Update();
}

void SDGUI::DoTabHY(Int_t id){
  hy_canvas[id]->GetCanvas()->Resize();
  hy_canvas[id]->GetCanvas()->Modified();
  hy_canvas[id]->GetCanvas()->Update();
}

void SDGUI::DoTabRD(Int_t id){
  rd_canvas[id]->GetCanvas()->Resize();
  rd_canvas[id]->GetCanvas()->Modified();
  rd_canvas[id]->GetCanvas()->Update();
}

void SDGUI::DoTabMD(Int_t id){
  md_canvas[id]->GetCanvas()->Resize();
  md_canvas[id]->GetCanvas()->Modified();
  md_canvas[id]->GetCanvas()->Update();
}

void SDGUI::EyeSelect(Int_t id){
  stringstream fd_name;
  stringstream hy_name;

  for (int i=0;i<Neyes;i++) {
    if (id==i) {
      fd_name << "fd" << i;
      hy_name << "hy" << i;
    }
  }

  fdEyeSelect->Select(id,kFALSE);
  fdEyeSelect->GetTextEntry()->SetText(fdEyeSelect->GetSelectedEntry()->GetTitle());
  TGListBox *fdEyeSelect_lb = fdEyeSelect->GetListBox();
  // update the scroll bar position
  ((TGLBContainer *)fdEyeSelect_lb->GetContainer())->SetVsbPosition(fdEyeSelect_lb->GetSelected());

  hyEyeSelect->Select(id,kFALSE);
  hyEyeSelect->GetTextEntry()->SetText(hyEyeSelect->GetSelectedEntry()->GetTitle());
  TGListBox *hyEyeSelect_lb = hyEyeSelect->GetListBox();
  // update the scroll bar position
  ((TGLBContainer *)hyEyeSelect_lb->GetContainer())->SetVsbPosition(hyEyeSelect_lb->GetSelected());

  fd_status = DrawRecPlots(fd_name.str(),fd_canvas);
  hy_status = DrawRecPlots(hy_name.str(),hy_canvas);
  UpdateTabColorStatus(fd_status,fd_tab,fd_objtabs);
  UpdateTabColorStatus(hy_status,hy_tab,hy_objtabs);
}

void SDGUI::EnergySelect(Int_t id){
  stringstream sd_name;

  for (int i=0;i<Nenergyrange;i++) {
    if (id==i)
      sd_name << "sd" << i;
  }

  sdEnergySelect->Select(id,kTRUE);
  sd_status = DrawRecPlots(sd_name.str(),sd_canvas);
  UpdateTabColorStatus(sd_status,sd_tab,sd_objtabs);
}

/*
Bool_t SDGUI::ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2)
{
  Pixel_t color;
  string plotname;
  switch(GET_MSG(msg)) {
  case kC_COMMAND:
    switch(GET_SUBMSG(msg)) {
    case kCM_BUTTON:
      switch(parm1) {
      case 1009:
	cout << "See you !" << endl;
	gApplication->Terminate(0);
	break;
      default:
	break;
      }
    }
  }
  return 0;
}
*/

int main(int argc, char **argv) 
{ 
  int *nTabs;

  // Check if the user specified the files to use
  if (argc == 3)
    obj_list = LoadData(argc, argv);
  else {
    Usage();
    return 0;
  }

  obj_list = AddAnalyzedObjects(obj_list);

  nTabs = GetNumberOfTabs(obj_list);
  
  sd_Ntab = nTabs[kSD];
  fd_Ntab = nTabs[kFD];
  hy_Ntab = nTabs[kHY];

  /*
  cout << "SD:  " << sd_Ntab << endl;
  cout << "FD:  " << fd_Ntab << endl;
  cout << "HY:  " << hy_Ntab << endl;
  obj_list->ls();
  */

  cout << endl << "Starting the DQM viewer... just press CTRL+C to exit." << endl << endl;

  // Running the App
  TApplication theApp("app", &argc, argv);
  SDGUI* display=new SDGUI(gClient->GetRoot(), 800, 600);
  theApp.Run();
}

string* GetParsedStrings(string str)
{
  int iString = 0;

  stringstream stream(str);
  string parsed_string;
  string* parsed = new string[sMax];
  while (getline(stream,parsed_string,'_')) {
    parsed[iString] = parsed_string;    
    iString++;
  }

  return parsed;
}

TList *LoadData(int argc, char **argv)
{
  stringstream ss;
  TH1 *histo;
  TGraph *graph;
  TList *list = new TList();

  // Getting the file names for later use
  refFile = argv[1];
  dqmFile = argv[2];

  string header[tMax] = {"ref","dqm"};

  for (int ifile=1;ifile<argc;ifile++) { 
    TFile *f;
    f = new TFile(argv[ifile]);
    TIter next(f->GetListOfKeys());
    TKey *key;
    while ( (key = (TKey*)next()) ) {
      TClass *cl = gROOT->GetClass(key->GetClassName());
      if (cl->InheritsFrom("TH1")) { 
	histo = (TH1*)key->ReadObj();
	ss << header[ifile-1] << "_" << histo->GetName();
	histo->SetName(ss.str().c_str());
	histo->SetDirectory(0);
	list->Add(histo);
      }
      else if (cl->InheritsFrom("TGraph")) {
	graph = (TGraph*)key->ReadObj();
	ss << header[ifile-1] << "_" << graph->GetName();
	graph->SetName(ss.str().c_str());
	list->Add(graph);
      }
      ss.str("");
    }
    f->Close();
  }
  
  return list;
}

int *GetNumberOfTabs(TList *list)
{
  int* nTabs = new int[tMax];
  string *parsed;
  string header;
  string stream;
  stringstream ss;

  for (int i=0; i<tMax; i++)
    nTabs[i] = 0;
  
  TObject *obj;
  TIter next(list);
  while ( (obj = next()) ) {    
    parsed = GetParsedStrings(obj->GetName());
    header = parsed[0];
    stream = parsed[1];

    if ( ((string)obj->ClassName() != "TGraph") && 
	 ((string)obj->ClassName() != "TGraphErrors") ) { // one counts the multigraphs, not the graphs
      if ( (header == "ref") || (header == "mgr") ) {
	if (stream == "sd0") nTabs[kSD]++;
	if (stream == "fd0") nTabs[kFD]++;
	if (stream == "hy0") nTabs[kHY]++;
      }
    }
  }
  
  return nTabs;
}

TList *AddAnalyzedObjects(TList *list)
{
  TObject *obj;
  TH1 *h1d_ref, *h1d_dqm, *h;
  TH2 *h2d_ref, *h2d_dqm;
  TGraph *g_ref, *g_dqm;
  TGraphErrors *ge_ref, *ge_dqm;
  TGraphErrors *ge;
  TMultiGraph *mg;
  string *parsed;
  string header;
  
  TIter next(list);
  while ( (obj = next()) ) {
    parsed = GetParsedStrings(obj->GetName());
    header = parsed[0];   
    if (header == "ref") {
      if ( ((string)obj->ClassName() == "TH1F") || ((string)obj->ClassName() == "TH1D") ) {
	h1d_ref = (TH1*)obj;
	h1d_dqm = (TH1*)FindMatchingDqmObject(obj, list);
 	ge = CreateResidualGraph(h1d_ref,h1d_dqm);
	list->Add(ge);
      }
      if ( ((string)obj->ClassName() == "TH2F") || ((string)obj->ClassName() == "TH2D") ) {
	h2d_ref = (TH2*)obj;
	h2d_dqm = (TH2*)FindMatchingDqmObject(obj, list);
	h = CreateConsistencyTest(h2d_ref,h2d_dqm);
	list->Add(h);
      }
      if ((string)obj->ClassName() == "TGraph") {
	g_ref = (TGraph*)obj;
	g_dqm = (TGraph*)FindMatchingDqmObject(obj, list);
	mg = CreateMultiGraph(g_ref,g_dqm);
	list->Add(mg);
      }
      if ((string)obj->ClassName() == "TGraphErrors") {
	ge_ref = (TGraphErrors*)obj;
	ge_dqm = (TGraphErrors*)FindMatchingDqmObject(obj, list);
	mg = CreateMultiGraph2(ge_ref,ge_dqm);
	list->Add(mg);
      }
    }
  }
 
  return list;
}

TObject *FindMatchingDqmObject(TObject *ref, TList *list)
{
  TObject *dqm;
  string *parsed;
  string header, stream, objectname;
  stringstream ss;
  
  parsed = GetParsedStrings(ref->GetName());
  header = parsed[0];
  stream = parsed[1];
  objectname = parsed[2];
 
  ss << "dqm_" << stream << "_" << objectname;

  dqm = (TObject*)list->FindObject(ss.str().c_str());

  return dqm;
}

TH1 *NormalizeToDQM(TH1 *ref, TH1 *dqm)
{
  if (ref->Integral()>0) {
    ref->Scale(dqm->Integral()/ref->Integral());
  }
  return ref;
}

TGraphErrors *CreateResidualGraph(TH1 *ref, TH1 *dqm)
{
  string *parsed;
  string header, stream, graphname;
  stringstream ss;

  parsed = GetParsedStrings(ref->GetName());
  header = parsed[0];
  stream = parsed[1];
  graphname = parsed[2];

  ss << "res_" << stream << "_" << graphname;

  TGraphErrors *ge = new TGraphErrors();
  ge->SetName(ss.str().c_str());
  ss.str("");

  int counter=0;  
  double refData, dqmData;
  double refDataErr, dqmDataErr;
  int Nbins = ref->GetNbinsX();

  ref = NormalizeToDQM(ref,dqm);

  for (int i=0; i<Nbins; i++) {
    refData = ref->GetBinContent(i);
    dqmData = dqm->GetBinContent(i);
    refDataErr = ref->GetBinError(i);
    dqmDataErr = dqm->GetBinError(i);
    if (refData!=0 || dqmData!=0) {
      ge->SetPoint(counter,ref->GetBinCenter(i),dqmData-refData);
      ge->SetPointError(counter,0,sqrt(pow(refDataErr,2)+pow(dqmDataErr,2)));
      counter++;
    }
  }

  ge->SetMarkerStyle(20);
  ge->SetMarkerSize(0.75);
  ge->SetLineWidth(2);
  ge->GetXaxis()->SetLimits(ref->GetXaxis()->GetXmin(),ref->GetXaxis()->GetXmax());
  ge->GetXaxis()->SetTitle(ref->GetXaxis()->GetTitle());
  ge->GetYaxis()->SetTitleOffset(0.9);
  ge->GetXaxis()->SetLabelFont(ref->GetXaxis()->GetLabelFont());
  ge->GetYaxis()->SetLabelFont(ref->GetYaxis()->GetLabelFont());
  ge->GetXaxis()->SetTitleFont(ref->GetXaxis()->GetTitleFont());
  ge->GetYaxis()->SetTitleFont(ref->GetYaxis()->GetTitleFont());
  ge->GetXaxis()->SetLabelSize(0.060);
  ge->GetXaxis()->SetTitleSize(0.060);
  ge->GetXaxis()->SetTitleOffset(1.0);
  ge->GetYaxis()->SetLabelSize(0.060);
  ge->GetYaxis()->SetTitleSize(0.060);

  return ge;
}

TMultiGraph *CreateMultiGraph(TGraph *ref, TGraph *dqm)
{
  string *parsed;
  string header, stream, graphname;
  stringstream ss;

  parsed = GetParsedStrings(ref->GetName());
  header = parsed[0];
  stream = parsed[1];
  graphname = parsed[2];

  ss << "mgr_" << stream << "_" << graphname;

  TMultiGraph *mg = new TMultiGraph();
  mg->SetName(ss.str().c_str());
  mg->SetTitle(ref->GetTitle());
  mg->Add(ref);
  mg->Add(dqm);

  return mg;
}

TMultiGraph *CreateMultiGraph2(TGraphErrors *ref, TGraphErrors *dqm)
{
  string *parsed;
  string header, stream, graphname;
  stringstream ss;

  parsed = GetParsedStrings(ref->GetName());
  header = parsed[0];
  stream = parsed[1];
  graphname = parsed[2];

  ss << "mgr_" << stream << "_" << graphname;

  TMultiGraph *mg = new TMultiGraph();
  mg->SetName(ss.str().c_str());
  mg->SetTitle(ref->GetTitle());
  mg->Add(ref);
  mg->Add(dqm);

  return mg;
}

TH1 *CreateConsistencyTest(TH2 *ref, TH2 *dqm)
{
  string *parsed;
  string header, stream, histoname;
  stringstream ss;

  parsed = GetParsedStrings(ref->GetName());
  header = parsed[0];
  stream = parsed[1];
  histoname = parsed[2];
  
  ss << "con_" << stream << "_" << histoname;

  TH1F *h = new TH1F(ss.str().c_str(),"Consistency test",100,-6,6);
  h->GetYaxis()->SetLimits(0.1,10000);

  return h;
}

void Usage()
{
  printf("\nUsage:\n"
  "-----\n\n"
  "./DQMviewer <reffile> <dqmfile>  \n"
  "where <reffile> and <dqmfile> are two ROOT files produced with CreatePeriodFile.\n"
  "\t <reffile> is the reference period considered. \n"
  "\t <dqmfile> is the (newer) period to be compared to the reference period. \n\n"
  "Questions / comments? Please contact Fred Sarazin: fsarazin@mines.edu \n\n");
}
