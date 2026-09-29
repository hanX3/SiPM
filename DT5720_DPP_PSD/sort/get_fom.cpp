

//
void get_fom_single(int run1, int run2)
{
  //
  TFile *fi1 = TFile::Open(TString::Format("run%04d_sort.root",run1).Data());
  if(!fi1 || fi1->IsZombie()){
    cout << "wrong open the file" << endl;
    return;
  }

  TTree *tr1 = (TTree*)fi1->Get("tr");
  if(!tr1) { std::cerr << "missing tree tr" << std::endl; return; }
  TH1D *h1 = new TH1D("h1", "h1", 10000, -100, 100);
  tr1->Draw("(qdc_long-qdc_short)/(qdc_long)>>h1", "qdc_long!=0");

  cout << h1->GetMaximum() << endl;
  cout << h1->GetMaximumBin() << endl;
  cout << h1->GetBinCenter(h1->GetMaximumBin()) << endl;

  Double_t max1 = h1->GetMaximum();
  if(max1 <= 0) { std::cerr << "empty PID histogram" << std::endl; return; }
  Double_t max_bin1 = h1->GetMaximumBin();

  //forward
  int i_forward1 = max_bin1;
  while(i_forward1 > 1){
    if(h1->GetBinContent(i_forward1)<max1/3.){
      break;
    }
    i_forward1--;
  }

  //backward
  int i_backward1 = max_bin1;
  while(i_backward1 < h1->GetNbinsX()){
    if(h1->GetBinContent(i_backward1)<max1/3.){
      break;
    }
    i_backward1++;
  }

  cout << "i_forward1 " << i_forward1 << endl;
  cout << "i_backward1 " << i_backward1 << endl;

  if(h1->Fit("gaus", "R", "", h1->GetBinCenter(i_forward1), h1->GetBinCenter(i_backward1)) != 0) {
    std::cerr << "Gaussian fit failed" << std::endl;
    return;
  }
  TF1 *fg1 = h1->GetFunction("gaus");
  if(!fg1) { std::cerr << "Gaussian fit failed" << std::endl; return; }

  cout << fg1->GetParameter(0) << endl;
  cout << fg1->GetParameter(1) << endl;
  cout << fg1->GetParameter(2) << endl;

  Double_t mean1 = fg1->GetParameter(1);
  Double_t sigma1 = fg1->GetParameter(2);

  //
  TFile *fi2 = TFile::Open(TString::Format("run%04d_sort.root",run2).Data());
  if(!fi2 || fi2->IsZombie()){
    cout << "wrong open the file" << endl;
    return;
  }

  TTree *tr2 = (TTree*)fi2->Get("tr");
  if(!tr2) { std::cerr << "missing tree tr" << std::endl; return; }
  TH1D *h2 = new TH1D("h2", "h2", 10000, -100, 100);
  tr2->Draw("(qdc_long-qdc_short)/(qdc_long)>>h2", "qdc_long!=0", "same");

  cout << h2->GetMaximum() << endl;
  cout << h2->GetMaximumBin() << endl;
  cout << h2->GetBinCenter(h2->GetMaximumBin()) << endl;

  Double_t max2 = h2->GetMaximum();
  if(max2 <= 0) { std::cerr << "empty PID histogram" << std::endl; return; }
  Double_t max_bin2 = h2->GetMaximumBin();

  //forward
  int i_forward2 = max_bin2;
  while(i_forward2 > 1){
    if(h2->GetBinContent(i_forward2)<max2/3.){
      break;
    }
    i_forward2--;
  }

  //backward
  int i_backward2 = max_bin2;
  while(i_backward2 < h2->GetNbinsX()){
    if(h2->GetBinContent(i_backward2)<max2/3.){
      break;
    }
    i_backward2++;
  }

  cout << "i_forward2 " << i_forward2 << endl;
  cout << "i_backward2 " << i_backward2 << endl;

  if(h2->Fit("gaus", "R", "", h2->GetBinCenter(i_forward2), h2->GetBinCenter(i_backward2)) != 0) {
    std::cerr << "Gaussian fit failed" << std::endl;
    return;
  }
  TF1 *fg2 = h2->GetFunction("gaus");
  if(!fg2) { std::cerr << "Gaussian fit failed" << std::endl; return; }

  cout << fg2->GetParameter(0) << endl;
  cout << fg2->GetParameter(1) << endl;
  cout << fg2->GetParameter(2) << endl;

  Double_t mean2 = fg2->GetParameter(1);
  Double_t sigma2 = fg2->GetParameter(2);

  const double width = std::abs(sigma1) + std::abs(sigma2);
  if(width <= 0) { std::cerr << "invalid Gaussian widths" << std::endl; return; }
  cout << std::abs(mean2-mean1)/width << endl;
}
