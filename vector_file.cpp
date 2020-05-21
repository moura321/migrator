#include <iostream>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <string.h>

using namespace std;

class Page
 {
      public:  
      unsigned long addr=0;
      int reads=0;
      int writes=0;
      int ifetch=0;
      char m_type;

      bool operator <(const Page & PgObj) const
        {return addr < PgObj.addr;}

      void set(unsigned long a, int r, int w, int i, char t)
      {
        addr=a;
        reads=r;
        writes=w;
        ifetch=i;
        m_type=t;
      } 
 };

int main()
{
    Page page;
    vector <Page> buffer;
    vector <vector <Page>> predict;

    Page max_read, max_write;
    ofstream out("arquvo.txt");

    FILE *arq;
    string line;

    char * ptr;
    char data[100];

    for(int i=0;i<3;i++)
    {
        for(int j=0;j<5;j++)
        {
            page.set(i,j,0,0,'c');
            buffer.push_back(page);
        }
        predict.push_back(buffer);
        buffer.clear();
    }
    
    for(int i = 0 ; i<predict.size(); i++)
    {
        for(int j = 0 ; j<predict[i].size(); j++)
        {
          if(predict[i][j].reads > max_read.reads)
          {
              max_read.addr=predict[i][j].addr;
              max_read.reads=predict[i][j].reads;
          }
          if(predict[i][j].writes > max_write.writes)
          {
              max_write.addr=predict[i][j].addr;
              max_write.writes=predict[i][j].writes;
          }
        }
        out << max_read.addr << ";" << max_read.reads << ";" << max_write.addr << ";" << max_write.writes <<"\n";
        max_read.reads=0;
        max_write.writes=0;
    }
    out.close();
    
    arq = fopen("arquvo.txt", "r");
    getline(arq,line);
    cout << ftell(arq);


/*     if (arq.is_open())
    {
     while (getline (arq,line))
     {
       for (int g = 0; g <= line.size(); g++)
           data[g] = line[g];
       ptr = strtok (data,";");
       cout << ptr << ".";
       ptr = strtok (NULL, ";");
       cout << ptr << ".";
       ptr = strtok (NULL, ";");
       cout << ptr << ".";
       ptr = strtok (NULL, ";");
       cout << ptr << "." <<endl;
     }
     arq.close();
    }
    else cout << "error opening file"; */
}