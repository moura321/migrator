#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <list>
#include <set>

#include <XmlRpcCpp.h>

#define NAME       "XML-RPC C++ Client"
#define VERSION    "0.1"
#define SERVER_URL "http://localhost:8080"

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



 class Hybrid_Memory
 {
  public:
  set <Page> mm; //Main Memory
  
  char type[2]={'D','P'}; //D=DRAM  S=SRAM  P=PCM  T=STTRAM  R=RRAM
  int size[2]={0,0}; //number of pages. Value 0 = unlimited
  int used[2]={0,0}; //pages on each memory module

  public:
  void insert_page(Page page)
  {
    mm.insert(page);
    for(int i=0; i < sizeof(type); i++)
      if(page.m_type==type[i])
      {
        used[i]++;
        break;
      }
  }

  bool erase_page(Page page)
  {
    
    for(int i=0; i < sizeof(type); i++)
      if(page.m_type==type[i])
      {
        used[i]--;
        break;
      } 
    return mm.erase(page);
  }
  
  set<Page>::iterator search_page(Page page)
    {
      return mm.find(page);
    }
  
  void move(Page page, char new_type)
  {
    Page temp_page;
    set<Page>::iterator it;
    
    it = search_page(page);
    temp_page = *it;
    
    temp_page.m_type = new_type;
    
    erase_page (*it);
    insert_page(temp_page);
    //cout << "\tmovi\n";//TODO retirar
  }

  void print()
  {
    set<Page>::iterator it;
    it = mm.begin();
    while(it != mm.end())
    {
      
      cout <<"addr:" << it->addr << "\t";
      cout <<"R:" <<  it->reads + it->ifetch << "\t";
      cout <<"W:" <<  it->writes << "\t";
      cout <<"T:" <<  it->m_type << "\n";
      it++;
    }
    std::cout << "\n";
  }
  
};


string buffer2string(vector <Page> *buffer)
{
  string result;
  for (int i=0; i<buffer->size(); i++)
  {
    result+= to_string(i) + ";"
    + to_string(buffer->operator[](i).addr) + ";"
    + to_string(buffer->operator[](i).reads + buffer->operator[](i).ifetch) + ";"
    + to_string(buffer->operator[](i).writes) + ";"
    + buffer->operator[](i).m_type + "\n";
  }
  return result;
}

class Migration {
  public:
  bool coin_migrator(int prob=10)
  {
      return (rand()%100 <= prob) ? true : false;
  }

  bool always_migrate()
  {
    return true;
  }

  

  vector <Page> fuzzy_recommendation(vector <Page> *buffer, int counters_size)//TODO implementar
  {
    vector <Page> recommendation;
    XmlRpcValue param_array = XmlRpcValue::makeArray();
    param_array.arrayAppendItem(XmlRpcValue::makeString(buffer2string(buffer)));
    param_array.arrayAppendItem(XmlRpcValue::makeInt(counters_size));
    param_array.arrayAppendItem(XmlRpcValue::makeInt(buffer->size()));

    XmlRpcClient server (SERVER_URL);
    XmlRpcValue result = server.call("fhm.promote", param_array);

    /*cout << result.structGetValue("addr").getInt() << endl;
    cout << result.structGetValue("value").getDouble() << endl;*/

    for(int i=0; i<result.structSize();i+=2)
      for(int j=0; j<buffer->size(); j++)
        if(buffer->operator[](j).addr == result.structGetValue(to_string(i)).getInt())
          recommendation.push_back(buffer->operator[](j));

    return recommendation;
  }

  void migrate_mem(vector <Page> pages2migrate, Hybrid_Memory *mem)
  {
    for (int i=0; i<pages2migrate.size(); i++)
      mem->move(pages2migrate[i], 'P');
  }

  void migrate_buffer(vector <Page> pages2migrate, vector <Page> *buffer)
  {
    for (int i=0; i<pages2migrate.size(); i++)
      for (int j=0; j<buffer->size(); j++)
        if(buffer->operator[](j).addr==pages2migrate[i].addr)
          buffer->operator[](j).m_type='P';
  }
};




//-------------------------------------------------------------------
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

void print_buffer_v(vector <Page> *pages)//verbose
{
  int i;
  //std::cout << "\n";
  for (i=0; i<pages->size(); i++)
  {
    std::cout << "[" << i << "]"
    <<"addr:" << pages->operator[](i).addr << "\t"
    <<"R:" <<  pages->operator[](i).reads + pages->operator[](i).ifetch << "\t"
    <<"W:" <<  pages->operator[](i).writes << "\t"
    <<"T:" <<  pages->operator[](i).m_type << "  ";
    std::cout << "\n";
  }
  std::cout << "\n";
}

void print_buffer(vector <Page> *buffer)
{
  std::cout << "PAGES\n";
  for (int i=0; i<buffer->size(); i++)
  {
    std::cout  << i << ";"
    << buffer->operator[](i).addr << ";"
    << buffer->operator[](i).reads + buffer->operator[](i).ifetch << ";"
    << buffer->operator[](i).writes << ";"
    << buffer->operator[](i).m_type << "\n";
  }
}



void rotate_buffer (vector <Page> *buffer, char op)
{
  for (int i=0; i<buffer->size(); i++)
  { 
    switch (op)
      {
      case 'R':
        buffer->operator[](i).reads/=2;
        break;

      case 'I':
        buffer->operator[](i).ifetch/=2;
        break;

      case 'W':
        buffer->operator[](i).writes/=2;
        break;
      }  
     
  }
}  




/*  int main()
 {
   Page page;
   Hybrid_Memory mm;

   page.set(2000, 8, 1, 2, 'V');
   mm.insert_page(page);

   page.set(2000, 1, 1, 2, 'D');
   mm.insert_page(page);

page.set(2000, 0, 1, 2, 'D');
   mm.insert_page(page);

   //cout << (*mm.search_page(page)).m_type << '\n';
   //mm.move(page, 'D');
   //cout << (*mm.search_page(page)).m_type << '\n';
   //cout << (mm.used[0]) << '\n';
   cout << mm.mm.size() << '\n';

   cout  << "show\n";
 } */



//#################################################


int main(int argc, char *argv[])
{
  const int DESLOC = 6;//6 bits para endereço, o resto para página TODO conferir esses valores
  const int PROB = 50;//probabilidade de migrar TODO tirar
  const bool LIMITED = (argc>2) ? (bool)atoi(argv[2]) : 1;//buffer tem limite? Por default, sim. 
  const int BUFFER = (argc>2) ? atoi(argv[2]): 8;//limite do buffer. Por default, 8
  const int TIME_TO_MIGRATE = (argc>3) ? atoi(argv[3]): 2;//migrar a cada quantas instrucoes? Por default, 4*buffer
  const bool DEBBUG = 1;
  const int COUNTERS_SIZE = 16;//contagem de reads e writes
  FILE *arq;
  char Linha[16];
  string l;
  int memory_accesses=0;

  unsigned long address;
  char op;
  int i=0;

  unsigned long max_address;
  unsigned long min_address;
  unsigned long max_page;
  unsigned long min_page;
  string max_hexa_address;
  string min_hexa_address;
  
  Hybrid_Memory mem;
  Page page;
  vector <Page> buffer;
  Migration m;
  
  unsigned long R_count=0;
  unsigned long W_count=0;
  unsigned long M_count=0;
  unsigned long I_count=0;
  unsigned long max_reads=0;
  unsigned long max_writes=0;

  int error_count=0;
  bool error=0;


  srand (time(NULL));
  XmlRpcClient::Initialize(NAME, VERSION);

  // Abre um arquivo TEXTO para LEITURA
  arq = fopen(argv[1], "rt");
  if (arq == NULL)  // Se houve erro na abertura
     printf("Problemas na abertura do arquivo\n");

  // Lê primeira linha e inicializa contadores e variaveis
  fgets(Linha, 16, arq);
  l=Linha;
  
  address = max_address = min_address = getAddress(l); 
  //max_hexa_address = min_hexa_address = getHexaAddress(l); //bom para debug
  min_page = max_page = getPage(address,DESLOC);

  page.set(getPage(address,DESLOC),0,0,0,'D');
  buffer.push_back(page);
  mem.insert_page(page);  
  

  while (!feof(arq))
  {
      l=Linha;
      
      address=getAddress(l); //endereço em (unsigned long)
      op = Linha[0]; //operação: R, W, M ou I

      page.set(getPage(address,DESLOC),0,0,0,'D'); //inicializa pagina temporaria

      i=0;
      while (buffer[i].addr != page.addr && i < buffer.size()) //procura pagina atual (page) no buffer (buffer)
        i++;

      if(i==buffer.size())//se nao encontrou a pagina no buffer
      {
        buffer.push_back(page);
        if(LIMITED)
          if(buffer.size()>BUFFER)
            buffer.erase(buffer.begin());
      }
      else
      {
        buffer.push_back(buffer[i]);
        buffer.erase(buffer.begin()+i);
      }
      
      i=buffer.size()-1; //i = posicao da pagina no vector pages

      mem.insert_page(buffer[i]);
      memory_accesses++;
      //TODO colocar um pontteiro pro buffer


  //---------------------------------------------------
  //--------------- Contadores ------------------------
  //---------------------------------------------------
      switch (op)
      {
      case 'R':
        buffer[i].reads++;
        R_count++;
        if(buffer[i].reads>COUNTERS_SIZE)
          rotate_buffer(&buffer, op);
        break;

      case 'I':
        buffer[i].ifetch++;
        I_count++;
        if(buffer[i].ifetch>COUNTERS_SIZE)
          rotate_buffer(&buffer, op);
        break;

      case 'W':
        buffer[i].writes+=10;
        W_count++;
        if(buffer[i].writes>COUNTERS_SIZE)
          rotate_buffer(&buffer, op);
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

      if(buffer[i].reads+buffer[i].ifetch > max_reads)
        max_reads=buffer[i].reads+buffer[i].ifetch;
      if(buffer[i].writes > max_writes)
        max_writes=buffer[i].writes;

      //Lê próxima linha (inclusive com o '\n')
      fgets(Linha, 16, arq);

      //TODO
      if (memory_accesses==TIME_TO_MIGRATE)
      {
        vector <Page> rec = m.fuzzy_recommendation(&buffer, COUNTERS_SIZE);
                     //DEBUGANDO ----------------------TODO
        if(DEBBUG)
        {
          cout << "Buffer:\n";
          print_buffer_v(&buffer);
          cout << "Recomenda migrar:\n";
          print_buffer_v(&rec);
        }
                      //--------------------------------

        m.migrate_mem(rec, &mem);
        m.migrate_buffer(rec, &buffer);
        memory_accesses=0;
      }


  }
  
  fclose(arq);

  //-------------prints
  //mem.print();
  //cout << buffer2string(&buffer);
  //print_buffer(&buffer);
  //fuzzy_promote(&buffer);
  //vector <Page> rec = m.fuzzy_recommendation(&buffer);
  //m.migrate_mem(rec, &mem);
  
  //mem.print();
 // cout << "mem normal" << endl;
 // mem.print();
  //cout << "Recomenda migrar" << endl;
 // cout << endl;
  print_buffer(&buffer);
  //m.migrate_buffer(rec, &buffer);
 // cout <<"Migrado" << endl;
  //mem.print();

  //print_diagnostico(buffer.size(), R_count, W_count, I_count, min_page, max_page, max_reads, max_writes);
}

