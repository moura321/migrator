#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <list>

using namespace std;

class Page
 {
   public:  
   unsigned long addr=0;
   int reads=0;
   int writes=0;
   int mem=0;
   int ifetch=0;
   char m_type;

   char last_op='N';//null

   int consecutivos=0;
   int consecutivos_r=0;
   int consecutivos_w=0;
   int consecutivos_m=0;
   int consecutivos_i=0;
   
   vector <int> comprimentos;
   vector <int> comprimentos_r;
   vector <int> comprimentos_w;
   vector <int> comprimentos_m;
   vector <int> comprimentos_i;
   
   public:
   void set_all(unsigned long a, int r, int w, int m, int i, char t)
   {
     addr=a;
     reads=r;
     writes=w;
     mem=m;
     ifetch=i;
     m_type = t;
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

void always_migrate(Page *pagina)
{
  pagina->m_type='N';
}

void coin_migrator(Page *pagina, int prob)
{
  if(rand() % 100<=prob)
      pagina->m_type=='N' ? pagina->m_type='V' : pagina->m_type='N';
}

unsigned long getAddress(string l)
{
  return (unsigned long)strtol(l.substr(3).c_str(), NULL, 16);
}

string getHexaAddress(string l)
{
  return l.substr(3);
}

unsigned long getPage(unsigned long address, int DESLOC)
{
  return address >> DESLOC;
}

int main(int argc, char *argv[])
{
  const int DESLOC = 6;
  const int probability = 50;

  FILE *arq;
  char Linha[16];
  string l;

  char *result;
  unsigned long address;
  char op;
  int last_page=-1;
  int i=0;

  unsigned long max_address;
  unsigned long min_address;
  unsigned long max_page;
  unsigned long min_page;
  string max_hexa_address;
  string min_hexa_address;
  
  Page page;
  list <Page> pages;
  list <Page> i;

  unsigned long R_count=0;
  unsigned long W_count=0;
  unsigned long M_count=0;
  unsigned long I_count=0;

  int error_count=0;
  bool error=0;


  srand (time(NULL));

  // Abre um arquivo TEXTO para LEITURA
  arq = fopen(argv[1], "rt");
  if (arq == NULL)  // Se houve erro na abertura
     printf("Problemas na abertura do arquivo\n");


  // Lê primeira linha e inicializa contadores e variaveis
  fgets(Linha, 16, arq);
  l=Linha;
  
  address = max_address = min_address = getAddress(l); 
  max_hexa_address = min_hexa_address = getHexaAddress(l);
  min_page = max_page = getPage(address,DESLOC);

  page.set_all(getPage(address,DESLOC),0,0,0,0,'X');
  pages.push_back(page);  


  while (!feof(arq))
  {
      l=Linha;
      
      address=getAddress(l); //endereço em (unsigned long)
      op = Linha[0]; //operação: R, W, M ou I

      page.set_all(getPage(address,DESLOC),0,0,0,0,'X');; //inicializa pagina temporaria
      

      i=0;
      while (pages[i].addr != page.addr && i < pages.size()) //procura pagina atual (page) na lista de paginas (pages)
        i++;
      if(i==pages.size())
        pages.push_back(page);

      //i = posicao da pagina no vector pages

  //---------------------------------------------------
  //--------------- Contadores ------------------------
  //---------------------------------------------------
      switch (op)
      {
      case 'I':
        pages[i].ifetch++;
        I_count++;
        break;

      case 'R':
        pages[i].reads++;
        R_count++;
        break;

      case 'W':
        pages[i].writes++;
        W_count++;
        break;

      case 'M':
        pages[i].mem++;
        M_count++;
        break;

      default:
        error_count++;
        break;
      }

      //Acessos consecutivos à mesma página
      if(last_page == i)
      {
        pages[i].comprimentos[pages[i].consecutivos]--;
        pages[i].consecutivos++;
        if(pages[i].consecutivos == pages[i].comprimentos.size())
          pages[i].comprimentos.push_back(1);
        else
          pages[i].comprimentos[pages[i].consecutivos]++;
      }
      else
      {
        pages[i].consecutivos=0;
        if(!pages[i].comprimentos.size())
          pages[i].comprimentos.push_back(1);
        else
          pages[i].comprimentos[0]++;
      }
      

      //Acessos consecutivos à mesma página com a e mesma operacao
      switch(op)
      {
      case 'R':
        if(last_page == i && pages[i].last_op == op)
          update_consecutivos(&pages[i].comprimentos_r, &pages[i].consecutivos_r);
        else
          nova_sequencia(&pages[i].comprimentos_r, &pages[i].consecutivos_r);          
        break;

      case 'W':
        if(last_page == i && pages[i].last_op == op)
          update_consecutivos(&pages[i].comprimentos_w, &pages[i].consecutivos_w);
        else
          nova_sequencia(&pages[i].comprimentos_w, &pages[i].consecutivos_w);          
        break;

      case 'M':
        if(last_page == i && pages[i].last_op == op)
          update_consecutivos(&pages[i].comprimentos_m, &pages[i].consecutivos_m);
        else
          nova_sequencia(&pages[i].comprimentos_m, &pages[i].consecutivos_m);          
        break;

      case 'I':
        if(last_page == i && pages[i].last_op == op)
          update_consecutivos(&pages[i].comprimentos_i, &pages[i].consecutivos_i);
        else
          nova_sequencia(&pages[i].comprimentos_i, &pages[i].consecutivos_i);          
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

  //------------------------------------------------------------
  //---------- Fim Contadores ----------------------------------
  //------------------------------------------------------------



  //------------------------------------------------------------
  //---------- Migração ----------------------------------------
  //------------------------------------------------------------


    //always_migrate(&pages[i]);
    //coin_migrator(&pages[i], probability);

    pages[i].last_op = op;
    last_page = i;
    //Lê próxima linha (inclusive com o '\n')
    fgets(Linha, 16, arq);
  }
  fclose(arq);



  //-------------prints

  std::cout << "PAGES\n";
  for (i=0; i<pages.size(); i++)
  {
    std::cout << "[" << i << "]"
    <<"addr:" << pages[i].addr << "  "
    <<"R:" <<  pages[i].reads << "  "
    <<"W:" <<  pages[i].writes << "  "
    <<"M:" <<  pages[i].mem << "  "
    <<"I:" <<  pages[i].ifetch << "  "
    <<"Ty:" <<  pages[i].m_type << "  ";
    /*
    <<"seq_r:" << pages[i].seq_r << "  "
    <<"seq_w:" << pages[i].seq_w << "  "
    <<"seq_m:" << pages[i].seq_m << "  "
    <<"seq_i:" << pages[i].seq_i;
    */
    
    /*
    std::cout << "\nR_freq ";
    for(int j=0; j < pages[i].comprimentos_r.size(); j++)
      std::cout << j+1 <<"[" << pages[i].comprimentos_r[j] << "] ";
    if(pages[i].comprimentos_r.size()>1)
      r++;

    std::cout << "\nW_freq ";
    for(int j=0; j < pages[i].comprimentos_w.size(); j++)
      std::cout << j+1 <<"[" << pages[i].comprimentos_w[j] << "] ";
    if(pages[i].comprimentos_w.size()>1)
      w++;

    std::cout << "\nM_freq ";
    for(int j=0; j < pages[i].comprimentos_m.size(); j++)
      std::cout << j+1 <<"[" << pages[i].comprimentos_m[j] << "] ";

    std::cout << "\nI_freq ";
    for(int j=0; j < pages[i].comprimentos_i.size(); j++)
      std::cout << j+1 <<"[" << pages[i].comprimentos_i[j] << "] ";
    if(pages[i].comprimentos_i.size()>1)
      f++;
     */
    std::cout << "\n";
    
  }

  std::cout << "\n\nDIAGNOSTICO " << '\n';

  std::cout << "Páginas únicas: " << pages.size() <<'\n';


  //std::cout << "Maior: int- "  << max_address << "  hexa- "<< max_hexa_address << "  page- " << max_page << '\n';
  //std::cout << "Menor: int- "  << min_address << "  hexa- "<< min_hexa_address <<  "  page- " << min_page  << '\n';
  std::cout << "R_count: "  << R_count << '\n';
  std::cout << "W_count: "  << W_count << '\n';
  std::cout << "M_count: "  << M_count << '\n';
  std::cout << "I_count: "  << I_count << '\n';
  std::cout << "Error1: "  << error_count << '\n';

}