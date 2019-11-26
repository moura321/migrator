#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <list>
#include <set>

using namespace std;

class Page
 {
   public:  
   unsigned long addr=0;
   int reads=0;
   int writes=0;
   int ifetch=0;
   char m_type;

   char last_op='N';//null

   public:
   void set_all(unsigned long a, int r, int w, int i, char t)
   {
     addr=a;
     reads=r;
     writes=w;
     ifetch=i;
     m_type = t;
   } 
 };

 class Hybrid_Memory
 {
  private:
  set <Page> mm; //Main Memory
  char type[2]={'D','P'}; //D=DRAM  S=SRAM  P=PCM  T=STTRAM  R=RRAM
  int max_size[2]={0,0}; //number of pages. Value 0 = unlimited
  int used[2]={0,0}; //pages on each memory module

  private:
  bool insert_page(Page page)
  {
    mm.insert(page);
    return true;
  }
  bool erase_page(Page page)
  {
    mm.erase(page);
    return true;
  }
  
  public:
  set<int>::iterator search_page(Page page)
  {
    return mm.find(page);
  }

  void move(Page page, char new_type)
  {
    Page temp_page;
    set<int>::iterator it;
    
    it = search_page(page);
    temp_page = *it;
    
    temp_page.mtype = new_type;
    
    erase_page (it);
    insert_page(temp_page);
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

void print_diagnostico (int tamanho, unsigned long R_count, unsigned long W_count, unsigned long I_count, unsigned long min_page, unsigned long max_page, unsigned long max_read, unsigned long max_write)
{
  std::cout << "\n\nDIAGNOSTICO " << '\n';
  std::cout << "Páginas:\t" << tamanho <<'\n';
  std::cout << "Maior pagina:\t" << max_page << '\n';
  std::cout << "Menor pagina:\t" << min_page  << '\n';
  std::cout << "R_count:\t"  << R_count+I_count << '\n';
  std::cout << "W_count:\t"  << W_count << '\n';
  std::cout << "Max read:\t"  << max_read << '\n';
  std::cout << "Max write:\t"  << max_write << '\n';

}


void print_pages(vector <Page> *pages)
{
  int i;
  std::cout << "PAGES\n";
  for (i=0; i<pages->size(); i++)
  {
    std::cout << "[" << i << "]"
    <<"addr:" << pages->operator[](i).addr << "\t"
    <<"R:" <<  pages->operator[](i).reads + pages->operator[](i).ifetch << "\t"
    <<"W:" <<  pages->operator[](i).writes << "\t"
    <<"T:" <<  pages->operator[](i).m_type << "  ";
    std::cout << "\n";
  }
}

void print_buffer(vector <Page> *pages)
{
  int i;
  std::cout << "PAGES\n";
  for (i=0; i<pages->size(); i++)
  {
    std::cout  << i << ";"
    << pages->operator[](i).addr << ";"
    << pages->operator[](i).reads + pages->operator[](i).ifetch << ";"
    << pages->operator[](i).writes << "\n";
  }
}

int main(int argc, char *argv[])
{
  const int DESLOC = 6;
  const int PROB = 50;
  const bool LIMITED = (argc>2) ? (bool)atoi(argv[2]) : 1;
  const int BUFFER = (argc>3) ? atoi(argv[3]): 5;
  
  
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
  vector <Page> pages;
  
  unsigned long R_count=0;
  unsigned long W_count=0;
  unsigned long M_count=0;
  unsigned long I_count=0;
  unsigned long max_reads=0;
  unsigned long max_writes=0;

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

  page.set_all(getPage(address,DESLOC),0,0,0,'V');
  pages.push_back(page);  


  while (!feof(arq))
  {
      l=Linha;
      
      address=getAddress(l); //endereço em (unsigned long)
      op = Linha[0]; //operação: R, W, M ou I

      page.set_all(getPage(address,DESLOC),0,0,0,'V');; //inicializa pagina temporaria

      i=0;
      while (pages[i].addr != page.addr && i < pages.size()) //procura pagina atual (page) na lista de paginas (pages)
        i++;

      if(i==pages.size())
      {
        pages.push_back(page);
        if(LIMITED)
          if(pages.size()>BUFFER)
            pages.erase(pages.begin());
      }
      else
      {
        pages.push_back(pages[i]);
        pages.erase(pages.begin()+i);
      }
      i=pages.size()-1; //i = posicao da pagina no vector pages

  //---------------------------------------------------
  //--------------- Contadores ------------------------
  //---------------------------------------------------
      switch (op)
      {
      case 'R':
        pages[i].reads++;
        R_count++;
        break;

      case 'I':
        pages[i].ifetch++;
        I_count++;
        break;

      case 'W':
        pages[i].writes++;
        W_count++;
        break;

      default:
        error_count++;
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

      if(pages[i].reads+pages[i].ifetch > max_reads)
        max_reads=pages[i].reads+pages[i].ifetch;
      if(pages[i].writes > max_writes)
        max_writes=pages[i].writes;


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
  //print_pages(&pages);
  print_buffer(&pages);
  print_diagnostico(pages.size(),R_count, W_count, I_count,min_page,max_page, max_reads, max_writes);
}

