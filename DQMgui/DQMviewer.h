
#include <iostream> // required for cout etc
#include <fstream>  // for reading in from / writing to an external file
#include <sstream> // string manipulation
#include <TGWindow.h>
#include <TGTab.h>
#include <TGLabel.h>
#include <TRootEmbeddedCanvas.h>
#include <TGStatusBar.h>
#include <TGComboBox.h>
#include <TObject.h>
#include <TH1.h>
#include <TH2.h>
#include <TGraphErrors.h>
#include <TF1.h>
#include <TList.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TMath.h>

using namespace std; // So the compiler interprets cout as std::cout

const int nHmax = 100; // Maximum of 100 off/mon Canvas... more than enough!

class SDGUI : public TGMainFrame {
private:
  TGHorizontalFrame *mainFrame;
  TGTab *tab;

  TGCompositeFrame *sd_frame;
  TGCompositeFrame *sd_objtabs[nHmax];
  TGTab *sd_tab;
  TGCompositeFrame *fd_frame;
  TGCompositeFrame *fd_objtabs[nHmax];
  TGTab *fd_tab;
  TGCompositeFrame *hy_frame;
  TGCompositeFrame *hy_objtabs[nHmax];
  TGTab *hy_tab;
  TGCompositeFrame *rd_frame;
  TGCompositeFrame *rd_objtabs[nHmax];
  TGTab *rd_tab;
  TGCompositeFrame *md_frame;
  TGCompositeFrame *md_objtabs[nHmax];
  TGTab *md_tab;

  // SD period frames
  TGCompositeFrame *sd_energy;
  TGCompositeFrame *sd_ref_frame;
  TGCompositeFrame *sd_dqm_frame;
  TGLabel *sd_ref_period;
  TGLabel *sd_dqm_period;  

  // FD eye selection + period frames
  TGCompositeFrame *fd_eyebox;
  TGCompositeFrame *fd_ref_frame;
  TGCompositeFrame *fd_dqm_frame;
  TGLabel *fd_ref_period;
  TGLabel *fd_dqm_period;

  // HY eye selection + period frames
  TGCompositeFrame *hy_eyebox;
  TGCompositeFrame *hy_ref_frame;
  TGCompositeFrame *hy_dqm_frame;
  TGLabel *hy_ref_period;
  TGLabel *hy_dqm_period;

  // RD period frames
  TGCompositeFrame *rd_ref_frame;
  TGCompositeFrame *rd_dqm_frame;
  TGLabel *rd_ref_period;
  TGLabel *rd_dqm_period;  

  // MD period frames
  TGCompositeFrame *md_ref_frame;
  TGCompositeFrame *md_dqm_frame;
  TGLabel *md_ref_period;
  TGLabel *md_dqm_period;  

  TRootEmbeddedCanvas *sd_canvas[nHmax];
  TRootEmbeddedCanvas *fd_canvas[nHmax];
  TRootEmbeddedCanvas *hy_canvas[nHmax];
  TRootEmbeddedCanvas *rd_canvas[nHmax];
  TRootEmbeddedCanvas *md_canvas[nHmax];

  // Status Bars
  TGStatusBar *fileStatusBar;
  TGStatusBar *sdStatusBar;
  TGStatusBar *fdStatusBar;
  TGStatusBar *hyStatusBar;
  TGStatusBar *rdStatusBar;
  TGStatusBar *mdStatusBar;
  
  TGCompositeFrame *sel_objtabs;

  // Combo Boxes
  TGComboBox *sdEnergySelect;
  TGComboBox *fdEyeSelect;
  TGComboBox *hyEyeSelect; 

  // Hints
  TGLayoutHints *hint_plots; 
  TGLayoutHints *hint_status;
  TGLayoutHints *hint_period;
  TGLayoutHints *hint_seltab;
  TGLayoutHints *hint;

public:
  SDGUI(const TGWindow *p, unsigned int w, unsigned int h);
  ~SDGUI();

  // Subroutines and Functions
  void ClearList();
  string GetPeriod(string filename);
  string *GetParsedStrings(string str);
  TGraph *GetGraphFromMultiGraph(TMultiGraph *mg, string header);
  TObject *FindMatchingDqmObject(TObject *ref, TList *list);
  TGraphErrors *FindMatchingResidual(TH1 *ref, TList *list);
  TH1 *FindMatchingConsistencyTest(TH2 *ref, TList *list);
  TH1 *ApplyHistogramStyle(TH1 *h);
  TGraph *ApplyGraphStyle(TGraph *g, string header);
  TGraphErrors *ApplyResidualStyle(TGraphErrors *ge, TH1 *histo1d_ref);
  TMultiGraph *ApplyMultiGraphStyle(TMultiGraph *mg);
  Pixel_t *DrawRecPlots(string selected_stream, TRootEmbeddedCanvas **canvas); 
  Pixel_t DrawHistogram1D(TH1* histo1d_ref, TRootEmbeddedCanvas *canvas);
  Pixel_t DrawHistogram2D(TH2* histo2d_ref, TRootEmbeddedCanvas *canvas);
  Pixel_t DrawMultiGraph(TMultiGraph *mg, TRootEmbeddedCanvas *canvas);
  //double meanLDFfunction(double *x, double *par);
  void DisplayLDFMultiGraphStat(TGraph *gREF, TGraph *gDQM, TF1 *fREF, TF1 *fDQM);
  void DisplayHistogramStat(TF1 *func);
  Pixel_t GetLDFMultiGraphStatus(TF1 *f,double min, double max);
  Pixel_t GetHistogramStatus(TF1 *func);
  void UpdateTabColorStatus(Pixel_t *status, TGTab *tab, TGCompositeFrame **htab);  
  //Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);

  // Slots
  void UpdateTab(int id, TRootEmbeddedCanvas **canvas); // not working yet
  void DoTab(Int_t id);
  void DoTabSD(Int_t id);
  void DoTabFD(Int_t id);
  void DoTabHY(Int_t id);
  void DoTabRD(Int_t id);
  void DoTabMD(Int_t id);
  void EyeSelect(Int_t id);
  void EnergySelect(Int_t id);

  ClassDef(SDGUI,0);
};
