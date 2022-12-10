
typedef struct {
  Double_t baseline;
  Double_t amplitude_max;
  Double_t energy_qdc;
  std::vector<Double_t> v_data;
} Data_Ana;

void test()
{
  Data_Ana data_ana;
  cout << "size " << sizeof(data_ana) << endl;

  Double_t s[100];
  cout << "size " << sizeof(s) << endl;

  return;
}