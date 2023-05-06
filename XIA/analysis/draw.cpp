
void draw()
{
  ifstream fi1;
  fi1.open("wave_run2.txt");
  ifstream fi2;
  fi2.open("wave_run3.txt");

  double t, data;
  int ltra = 0;

  vector<double> v_t1;
  vector<double> v_data1;
  while(1){
    fi1 >> t >> data;
    if(!fi1.good()) break;
    v_t1.push_back(t);
    v_data1.push_back(data);
    ltra++;
  }

  cout << ltra << endl;
  TH1D *h1 = new TH1D("h1", "", ltra, 0, ltra);
  for(int i=0;i<ltra;i++){
    h1->SetBinContent(i+1, v_data1[i]);
  }

  //
  ltra = 0;
  vector<double> v_t2;
  vector<double> v_data2;
  while(1){
    fi2 >> t >> data;
    if(!fi2.good()) break;
    v_t2.push_back(t);
    v_data2.push_back(data);
    ltra++;
  }

  cout << ltra << endl;
  TH1D *h2 = new TH1D("h2", "", ltra, 0, ltra);
  for(int i=0;i<ltra;i++){
    h2->SetBinContent(i+1, v_data2[i]);
  }
  auto cc = new TCanvas();
  cc->cd();

  h1->SetLineColor(2);
  h1->Draw();
  h2->SetLineColor(4);
  h2->Draw("same");

  //
  double baseline1 = 0;
  double baseline2 = 0;
  for(int i=0;i<400;i++){
    baseline1 += v_data1[i];
    baseline2 += v_data2[i];
  }
  baseline1 /= 400.;
  baseline2 /= 400.;

  for(int i=0;i<ltra;i++){
    v_data1[i] -= baseline1;
    v_data2[i] -= baseline2;
  }

  double max1 = *max_element(v_data1.begin(), v_data1.end());
  double max2 = *max_element(v_data2.begin(), v_data2.end());
  cout << "max1 " << max1 << endl;
  cout << "max2 " << max2 << endl;

  TH1D *hn1 = new TH1D("hn1", "", ltra, 0, ltra);
  for(int i=0;i<ltra;i++){
    hn1->SetBinContent(i+1, v_data1[i]/max1);
  }

  TH1D *hn2 = new TH1D("hn2", "", ltra, 0, ltra);
  for(int i=0;i<ltra;i++){
    hn2->SetBinContent(i+1, v_data2[i]/max2);
  }

  auto ccn = new TCanvas();
  ccn->cd();

  hn1->SetLineColor(2);
  hn1->Draw();
  hn2->SetLineColor(4);
  hn2->Draw("same");
}
