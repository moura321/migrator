#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>

using namespace std;


class Page
 {
   public:  
   unsigned long addr=0;
   mutable int reads=0;
   mutable int writes=0;
   mutable int mem=0;
   mutable int ifetch=0;

   mutable char last_op='N';//null
   mutable int seq_op=0;
   mutable int seq_r=0;
   mutable int seq_w=0;
   mutable int seq_m=0;
   mutable int seq_i=0;

   mutable int consecutivos=0;
   mutable int consecutivos_r=0;
   mutable int consecutivos_w=0;
   mutable int consecutivos_m=0;
   mutable int consecutivos_i=0;
   
   mutable vector <int> comprimentos;
   mutable vector <int> comprimentos_r;
   mutable vector <int> comprimentos_w;
   mutable vector <int> comprimentos_m;
   mutable vector <int> comprimentos_i;
   
   public:
   void set_all(unsigned long a, int r, int w, int m, int i)
   {
     addr=a;
     reads=r;
     writes=w;
     mem=m;
     ifetch=i;

   } 
 };


void update_consecutivos(vector <int> *comprimentos, int *consecutivos)
{
  comprimentos->operator[](*consecutivos)--;
  (*consecutivos)++;
  if(*consecutivos == comprimentos->size())
    comprimentos->push_back(1);
  else
    comprimentos->operator[](*consecutivos)++;
}

void nova_sequencia(vector <int> *comprimentos, int *consecutivos)
{
  *consecutivos=0;
  if(!comprimentos->size())
    comprimentos->push_back(1);
  else
    comprimentos->operator[](0)++;
}



int main(int argc, char *argv[])
{
  const int DESLOC = 6;

  FILE *arq;
  char Linha[16];
  string l;

  char *result;
  unsigned long address;
  char op;
  int i=0;

  unsigned long max_address;
  unsigned long min_address;
  unsigned long max_page;
  unsigned long min_page;
  string max_hexa_address;
  string min_hexa_address;
  
  Page page;
  set<Page> pages;
  set<Page>::iterator it;
  set<Page>::iterator last_page;
  
  unsigned long R_count;
  unsigned long W_count;
  unsigned long M_count;
  unsigned long I_count;

  int error_count;
  bool error=0;
  int r=0;
  int w=0;
  int f=0;



  // Abre um arquivo TEXTO para LEITURA
  arq = fopen(argv[1], "rt");
  if (arq == NULL)  // Se houve erro na abertura
     printf("Problemas na abertura do arquivo\n");



  //-------------------------------------inicializa contadores e variaveis
  // Lê uma linha (inclusive com o '\n')
  fgets(Linha, 16, arq);
  l=Linha;
  
  address = max_address = min_address = (unsigned long)strtol(l.substr(3).c_str(), NULL, 16);
  max_hexa_address = min_hexa_address = l.substr(3);
  min_page = max_page = address >> DESLOC;

  //page.set_all(address >> DESLOC,0,0,0,0);
  //pages.insert(page);  
  R_count = 0;
  W_count = 0;
  M_count = 0;
  I_count = 0;
  error_count = 0;

  while (!feof(arq))
  {
      l=Linha;
      
      address=(unsigned long)strtol(l.substr(3).c_str(), NULL, 16); //endereço em (unsigned long)
      op = Linha[0]; //operação: R, W, M ou I

      page.set_all(address >> DESLOC,0,0,0,0); //inicializa pagina temporaria
      

      it=pages.insert(page);



      switch (op)
      {
      case 'I':
        it->ifetch++;
        I_count++;
        break;

      case 'R':
        it->reads++;
        R_count++;
        break;

      case 'W':
        it->writes++;
        W_count++;
        break;

      case 'M':
        it->mem++;
        M_count++;
        break;

      default:
        error_count++;
        break;
      }


      //operacao
      if (it->last_op == op)
      {
        it->seq_op++;
        switch (op)
        {
        case 'I':
          if(it->seq_op > it->seq_i) it->seq_i = it->seq_op;
          break;

        case 'R':
          if(it->seq_op > it->seq_r) it->seq_r = it->seq_op;
          break;

        case 'W':
          if(it->seq_op > it->seq_w) it->seq_w = it->seq_op;
          break;

        case 'M':
          if(it->seq_op > it->seq_m) it->seq_m = it->seq_op;
          break;

        default:
          error_count++;
          break;
        }     
      }
      else
        it->seq_op=1;


      //Consecutivos
      if(last_page == it)
      {
        it->comprimentos[it->consecutivos]--;
        it->consecutivos++;
        if(it->consecutivos == it->comprimentos.size())
          it->comprimentos.insert(1);
        else
          it->comprimentos[it->consecutivos]++;
      }
      else
      {
        it->consecutivos=0;
        if(!it->comprimentos.size())
          it->comprimentos.insert(1);
        else
          it->comprimentos[0]++;
      }
      


      //Consecutivos e mesma operacao
      switch(op)
      {
      case 'R':
        if(*last_page == *it && it->last_op == op)
          update_consecutivos(&it->comprimentos_r, &it->consecutivos_r);
        else
          nova_sequencia(&it->comprimentos_r, &it->consecutivos_r);          
        break;

      case 'W':
        if(*last_page == *it && it->last_op == op)
          update_consecutivos(&it->comprimentos_w, &it->consecutivos_w);
        else
          nova_sequencia(&it->comprimentos_w, &it->consecutivos_w);          
        break;

      case 'M':
        if(*last_page == *it && it->last_op == op)
          update_consecutivos(&it->comprimentos_m, &it->consecutivos_m);
        else
          nova_sequencia(&it->comprimentos_m, &it->consecutivos_m);          
        break;

      case 'I':
        if(*last_page == *it && it->last_op == op)
          update_consecutivos(&it->comprimentos_i, &it->consecutivos_i);
        else
          nova_sequencia(&it->comprimentos_i, &it->consecutivos_i);          
        break;

      default:
        break;
      }




      //Menores e maiores
      if(address>max_address)
      {
        max_address=address;
        max_hexa_address = l.substr(3);
        max_page = page.addr;
      }
      if(address<min_address)
      {
        min_address=address;
        min_hexa_address = l.substr(3);
        min_page = page.addr;
      }


    it->last_op = op;
    last_page = it;
    //Lê próxima linha (inclusive com o '\n')
    fgets(Linha, 16, arq);
  }
  fclose(arq);



  //-------------prints

  std::cout << "PAGES\n";
  for (i=0; i<pages.size(); i++)
  {
    std::cout << "[" << i << "]"
    <<"addr:" << it->addr << "  "
    <<"R:" <<  it->reads << "  "
    <<"W:" <<  it->writes << "  "
    <<"M:" <<  it->mem << "  "
    <<"I:" <<  it->ifetch << "  "
    <<"seq_r:" << it->seq_r << "  "
    <<"seq_w:" << it->seq_w << "  "
    <<"seq_m:" << it->seq_m << "  "
    <<"seq_i:" << it->seq_i;

    std::cout << "\nR_freq ";
    for(int j=0; j < it->comprimentos_r.size(); j++)
      std::cout << j+1 <<"[" << it->comprimentos_r[j] << "] ";
    if(it->comprimentos_r.size()>1)
      r++;

    std::cout << "\nW_freq ";
    for(int j=0; j < it->comprimentos_w.size(); j++)
      std::cout << j+1 <<"[" << it->comprimentos_w[j] << "] ";
    if(it->comprimentos_w.size()>1)
      w++;

    std::cout << "\nM_freq ";
    for(int j=0; j < it->comprimentos_m.size(); j++)
      std::cout << j+1 <<"[" << it->comprimentos_m[j] << "] ";

    std::cout << "\nI_freq ";
    for(int j=0; j < it->comprimentos_i.size(); j++)
      std::cout << j+1 <<"[" << it->comprimentos_i[j] << "] ";
    if(it->comprimentos_i.size()>1)
      f++;

    std::cout << "\n\n";

  }

  std::cout << "\n\nDIAGNOSTICO " << r << " " << w << " " << f << '\n';

  std::cout << "Páginas únicas: " << pages.size() <<'\n';


  //std::cout << "Maior: int- "  << max_address << "  hexa- "<< max_hexa_address << "  page- " << max_page << '\n';
  //std::cout << "Menor: int- "  << min_address << "  hexa- "<< min_hexa_address <<  "  page- " << min_page  << '\n';
  std::cout << "R_count: "  << R_count << '\n';
  std::cout << "W_count: "  << W_count << '\n';
  std::cout << "M_count: "  << M_count << '\n';
  std::cout << "I_count: "  << I_count << '\n';
  std::cout << "Error: "  << error_count << '\n';

}