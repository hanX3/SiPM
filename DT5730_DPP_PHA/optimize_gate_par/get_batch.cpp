
void get_batch()
{
  ofstream fo;
  fo.open("batch.sh");

  fo << "#!/usr/bin/bash" << endl;
  fo << endl;

  for(int lg1=400;lg1<=2000;lg1+=100){
    for(int lg2=400;lg2<=2000;lg2+=100){
      for(int sg1=400;sg1<=2000;sg1+=100){
        for(int sg2=400;sg2<=2000;sg2+=100){
          if(lg1>=lg2 || sg1>=sg2) continue;
          if(lg1==sg1 && lg2==sg2) continue;
          fo << "./sort 3 " << lg1 << " " << lg2 << " " << sg1 << " " << sg2 << endl;
          fo << "./sort 11 " << lg1 << " " << lg2 << " " << sg1 << " " << sg2 << endl;
        }
      }
    }
  }

  fo.close();
}