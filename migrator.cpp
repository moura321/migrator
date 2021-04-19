#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <fstream>
#include <string.h>


#include <XmlRpcCpp.h>

#define NAME       "XML-RPC C++ Client"
#define VERSION    "0.1"
#define SERVER_URL "http://localhost:8080"

using namespace std;


class Page
 {
      public:  
      long addr=0;
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
      int promoted=0, demoted=0;

      unsigned long int count_r_D=0, count_r_P=0, count_w_D=0, count_w_P=0;

      public:
      void insert_page(Page page, char op)
      {
        pair<set<Page>::iterator, bool> p = mm.insert(page); 
        if (p.second)
          for(int i=0; i < sizeof(type); i++)
            if(page.m_type==type[i])
            {
              used[i]++;
              break;
            }
        if(op=='R' && page.m_type=='D')
          count_r_D++;
        if(op=='R' && page.m_type=='P')
          count_r_P++;
        if(op=='W' && page.m_type=='D')
          count_w_D++;
        if(op=='W' && page.m_type=='P')
          count_w_P++;
      }

      bool erase_page(Page page)
      {
        for(int i=0; i < sizeof(type); i++)
          if(page.m_type==type[i])
          {
            used[i]--;
            break;
          } 
        if(page.m_type=='D')
          count_r_D++;
        if(page.m_type=='P')
          count_r_P++;
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
        insert_page(temp_page, 'W');

        //cout << "Page: " << page.addr << page.m_type << " newP>" << temp_page.addr << temp_page.m_type << endl;

        for(int i=0; i < sizeof(type); i++)  // TODO: with more than 2 types we'll have a problem
          if(new_type==type[i])
            used[i]++;
          else
            used[i]--;
      }


      char getm_type (Page page)
      {
        Page temp_page;
        set<Page>::iterator it;
        
        it = search_page(page);
        if(it != mm.end())
        {
          temp_page = *it;
          return temp_page.m_type;
        }
        else
          return '\0';
      }

      void print()
      {
        set<Page>::iterator it;
        it = mm.begin();
        while(it != mm.end())
        {
          cout <<"addr:" << it->addr << "\t";
          //cout <<"R:" <<  it->reads + it->ifetch << "\t";
          //cout <<"W:" <<  it->writes << "\t";
          cout <<"T:" <<  it->m_type << "\n";
          it++;
        }
      }

      void print_stats()
      {
        cout <<"--\n";
        cout <<"Promoted: \t" << promoted <<'\n';
        cout <<"Demoted: \t" << demoted <<'\n';
        cout <<"--\n";
        cout <<"Writes DRAM: \t" << count_w_D <<'\n';
        cout <<"Writes PCM: \t" << count_w_P <<'\n';
        cout <<"Total writes: \t" << count_w_D + count_w_P <<'\n';
        cout <<"--\n";
        cout <<"Reads on DRAM: \t" << count_r_D <<'\n';
        cout <<"Reads on PCM: \t" << count_r_P <<'\n';
        cout <<"Total reads: \t" << count_r_D + count_r_P <<'\n';
        cout <<"--\n";
        cout <<"Pages on memory: \t" << mm.size() <<'\n';   
        cout <<'\n';
      }

      void print_stats_clean()
      {
        cout << promoted <<";";
        cout << demoted <<";";
        cout << count_w_D <<";";
        cout << count_w_P <<";";
        cout << count_w_D + count_w_P <<";";
        cout << count_r_D <<";";
        cout << count_r_P <<";";
        cout << count_r_D + count_r_P <<";";
        cout << mm.size() <<";";
        //cout <<'\n';
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
      double promote_value;// values gratter than that will recommend promotion
      double demote_value;// values lower than that will recommend demotion
      
      bool coin(int prob=10)
      {
          return (rand()%100 <= prob) ? true : false;
      }

      vector <Page> always_migrate(vector <Page> *buffer)
      {
        vector <Page> recommendation;
        for(int i=0; i<buffer->size(); i++)
          recommendation.push_back(buffer->operator[](i));
        return recommendation;
      }
   


        vector <Page> fuzzy_AO(vector <Page> *buffer, int counters_size, int buffer_size, bool debbug)
      {
        int address;
        double f_value;
        vector <Page> recommendation;

        XmlRpcValue param_array = XmlRpcValue::makeArray();
        param_array.arrayAppendItem(XmlRpcValue::makeString(buffer2string(buffer)));
        param_array.arrayAppendItem(XmlRpcValue::makeInt(counters_size));
        param_array.arrayAppendItem(XmlRpcValue::makeInt(buffer_size));

        XmlRpcClient server (SERVER_URL);
        XmlRpcValue result = server.call("fhm.promote", param_array);

        for(int i=0; i<result.structSize();i+=2)
        {
          address = result.structGetValue(to_string(i)).getInt();
          f_value = result.structGetValue(to_string(i+1)).getDouble();
        }
        return recommendation;
      }





      vector <Page> fuzzy_recommendation(vector <Page> *buffer, int counters_size, int buffer_size, bool debbug)
      {
        int address;
        double f_value;
        vector <Page> recommendation;

        XmlRpcValue param_array = XmlRpcValue::makeArray();
        param_array.arrayAppendItem(XmlRpcValue::makeString(buffer2string(buffer)));
        param_array.arrayAppendItem(XmlRpcValue::makeInt(counters_size));
        param_array.arrayAppendItem(XmlRpcValue::makeInt(buffer_size));

        XmlRpcClient server (SERVER_URL);
        XmlRpcValue result = server.call("fhm.promote", param_array);

        for(int i=0; i<result.structSize();i+=2)
        {
          address = result.structGetValue(to_string(i)).getInt();
          f_value = result.structGetValue(to_string(i+1)).getDouble();

          for(int j=0; j<buffer->size(); j++)
            if(buffer->operator[](j).addr == address &&
                (
                  (buffer->operator[](j).m_type == 'P'  &&  f_value > promote_value)
                  ||
                  (buffer->operator[](j).m_type == 'D'  &&  f_value < demote_value)
                )
              )
              {
                recommendation.push_back(buffer->operator[](j));
                if(debbug)
                {
                  //cout << buffer->operator[](j).addr << " " << f_value << "\n";
                }

              }
        }
        return recommendation;
      }

      vector <Page> coin_recommendation(Page p, char op)
      {
        vector <Page> recommendation;
        if(coin(20))
          if(  (p.m_type=='D' && op =='R')  ||  (p.m_type=='P' && op =='W')  )
            recommendation.push_back(p);
        return recommendation;
      }
      
      vector <Page> oracle_predictor(ifstream &memory, Hybrid_Memory *mem)
      {
        vector <Page> recommendation;
        Page max_read, max_write;
        int diff_read, diff_write;
        string line;
        char data[100];
        char * ptr;

        int threshold = 0;

        Page temp_page;
        set<Page>::iterator it;

   
        if(getline (memory,line))
        {
          for (int g = 0; g <= line.size(); g++)
            data[g] = line[g];
          
          
          ptr = strtok (data,";");//read addr
          max_read.addr = atoi(ptr);
          //cout << max_read.addr << ";";

          ptr = strtok (NULL, ";");//read count
          diff_read=atoi(ptr);
          //cout << diff_read << ";";

          ptr = strtok (NULL, ";");//write addr
          max_write.addr = atoi(ptr);
          //cout << max_write.addr << ";";

          ptr = strtok (NULL, ";");//write count
          diff_write=atoi(ptr);
          //cout << diff_write << ";" << endl;
          
          //cout << "if:" << diff_read << " and " << max_read.addr <<endl;
          if(diff_read > threshold && max_read.addr > -1)
          {
            it = mem->search_page(max_read);
            temp_page = *it;
            if(it != mem->mm.end() && temp_page.addr == max_read.addr && temp_page.m_type=='D')
              recommendation.push_back(temp_page);
            //cout << "Demoting: " << temp_page.addr << ";" << diff_read << ";" << temp_page.m_type << endl;
          }

          //cout << "if:" << diff_write << " and " << max_write.addr <<endl;
          if(diff_write > threshold && max_write.addr > -1)
          {
            it = mem->search_page(max_write);
            temp_page = *it;
            if(it != mem->mm.end() && temp_page.addr == max_write.addr && temp_page.m_type=='P')
              recommendation.push_back(temp_page);
            //cout << "Promoting: " << temp_page.addr << ";" << diff_write << ";" << temp_page.m_type << endl;
          } 
        }
        return recommendation;
      }

      void migrate_mem(vector <Page> pages2migrate, Hybrid_Memory *mem)
      {
        for (int i=0; i<pages2migrate.size(); i++)
          if(pages2migrate[i].m_type=='D')
          {
            mem->move(pages2migrate[i], 'P');
            mem->demoted++;
          }
          else
          {
            mem->move(pages2migrate[i], 'D');
            mem->promoted++;
          }
      }

      void migrate_buffer(vector <Page> pages2migrate, vector <Page> *buffer)
      {
        for (int i=0; i<pages2migrate.size(); i++)
          for (int j=0; j<buffer->size(); j++)
            if(buffer->operator[](j).addr==pages2migrate[i].addr)
              if(buffer->operator[](j).m_type=='P')
                buffer->operator[](j).m_type='D';
              else
                buffer->operator[](j).m_type='P';
      }
};




//-------------------------------------------------------------------
unsigned long getAddress(string l)
{
  return (unsigned long)strtol(l.substr(2).c_str(), NULL, 16);
}

string getHexaAddress(string l)
{
  return l.substr(2);
}

unsigned long getPage(unsigned long address, int DESLOC)
{
  return address >> DESLOC;
}

void print_diagnostico(int tamanho, unsigned long R_count, unsigned long W_count, unsigned long I_count, unsigned long min_page, unsigned long max_page, unsigned long max_read, unsigned long max_write)
{
  cout << "\n\nDIAGNOSTICO " << '\n';
  cout << "Páginas:\t" << tamanho <<'\n';
  cout << "Maior pagina:\t" << max_page << '\n';
  cout << "Menor pagina:\t" << min_page  << '\n';
  cout << "R_count:\t"  << R_count+I_count << '\n';
  cout << "W_count:\t"  << W_count << '\n';
  cout << "Max read:\t"  << max_read << '\n';
  cout << "Max write:\t"  << max_write << '\n';
}

void print_buffer_v(vector <Page> *pages) //verbose
{
  int i;
  for (i=0; i<pages->size(); i++)
  {
    cout << "[" << i << "]"
    <<"addr:" << pages->operator[](i).addr << "\t"
    <<"R:" <<  pages->operator[](i).reads + pages->operator[](i).ifetch << "\t"
    <<"W:" <<  pages->operator[](i).writes << "\t"
    <<"T:" <<  pages->operator[](i).m_type << "  ";
    cout << "\n";
  }
  cout << "\n";
}

void print_buffer(vector <Page> *buffer)
{
  cout << "PAGES\n";
  for (int i=0; i<buffer->size(); i++)
  {
    cout  << i << ";"
    << buffer->operator[](i).addr << ";"
    << buffer->operator[](i).reads + buffer->operator[](i).ifetch << ";"
    << buffer->operator[](i).writes << ";"
    << buffer->operator[](i).m_type << "\n";
  }
}

void rotate_buffer (vector <Page> *buffer, char op)
{
  for (int i=0; i<buffer->size(); i++)
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


int main(int argc, char *argv[])
{
  // arq.tr buffer_size counter_size time2migrate promo demo
  //coin: arq.tr 1 1 1
  const char M_POLICY = (argc>2) ? *argv[2] : 'c';//a=always f=fuzzy c=coin o=oracle e=oracle eye
  const bool LIMITED = (argc>3) ? (bool)atoi(argv[3]) : 0;//buffer tem limite? Por default, não.
  const bool DEBBUG = 0;

  const int DESLOC = 6;//6 bits para endereço, o resto para página TODO conferir esses valores
  const int BUFFER_SIZE = (argc>3) ? atoi(argv[3]): 32;//limite do buffer. ultimo valor é default
  const int COUNTERS_SIZE = (argc>4) ? atoi(argv[4]): 32;//contagem de reads e writes. ultimo valor é default
  const int TIME_TO_MIGRATE = (argc>5) ? atoi(argv[5]): 16;//migrar a cada quantas instrucoes? ultimo valor é default
  const double PROMOTE_VALUE=(argc>6) ? atof(argv[6]): 4;
  const double DEMOTE_VALUE=(argc>7) ? atof(argv[7]): 2;
  
  
  FILE *arq;
  char Linha[16];
  string l;
  int memory_accesses=0;

  unsigned long address;  
  char op;
  int i=0;
  char m_type;

  unsigned long max_address;
  unsigned long min_address;
  unsigned long max_page;
  unsigned long min_page;
  string max_hexa_address;
  string min_hexa_address;
  
  Hybrid_Memory mem;
  Page page;
  Page temp_page;
  vector <Page> buffer;
  vector <Page> rec;
  vector <vector <Page>> predict;//vector of buffers
  Migration m;

  
  unsigned long R_count=0;
  unsigned long W_count=0;
  unsigned long M_count=0;
  unsigned long I_count=0;
  unsigned long max_reads=0;
  unsigned long max_writes=0;

  int error_count=0;
  bool error=0;

  m.promote_value=PROMOTE_VALUE;
  m.demote_value=DEMOTE_VALUE;

  srand (time(NULL));
  XmlRpcClient::Initialize(NAME, VERSION);

 ifstream oracle_memory_in;
 oracle_memory_in.open("oracle_memory.dat");
 if(M_POLICY=='o')//init oracle
 {
   string line;
   getline (oracle_memory_in,line);
 }
  

  // Abre arquivo trace
  arq = fopen(argv[1], "rt");
  if (arq == NULL)  // Se houve erro na abertura
     printf("Problemas na abertura do arquivo\n");

  // Lê primeira linha e inicializa contadores e variaveis
  fgets(Linha, 16, arq);
  l=Linha;
  
  address = max_address = min_address = getAddress(l); 

  //max_hexa_address = min_hexa_address = getHexaAddress(l); //bom para debug
  min_page = max_page = getPage(address,DESLOC);
 // op = Linha[0];
 // page.set(getPage(address,DESLOC),0,0,0,'D');
 // buffer.push_back(page);
 // mem.insert_page(page, op);  
 

  while (!feof(arq))
  {
      l=Linha;
      address=getAddress(l); //endereço em (unsigned long)
      op = Linha[0]; //operação: R, W

      page.set(getPage(address,DESLOC),0,0,0,'D'); //inicializa pagina temporaria

      m_type = mem.getm_type(page);
      if (m_type != '\0')
        page.m_type = m_type;

      i=0;
      while (i < buffer.size() && buffer[i].addr != page.addr) //procura pagina atual (page) no buffer (buffer)
        i++;

      if(i==buffer.size())//se nao encontrou a pagina no buffer
      {
        buffer.push_back(page);
        if(LIMITED)
          if(buffer.size()>BUFFER_SIZE)
            buffer.erase(buffer.begin());
      }
      else
      {
        buffer.push_back(buffer[i]);
        buffer.erase(buffer.begin()+i);
      }
      
      i=buffer.size()-1; //i = posicao da pagina no vector pages

      mem.insert_page(buffer[i], op);
      memory_accesses++;

  //---------------------------------------------------
  //--------------- Contadores ------------------------
  //---------------------------------------------------
      switch (op)
      {
      case 'R':
        buffer[i].reads++;
        R_count++;

/*         if(buffer[i].m_type=='D')
          mem.count_r_D++;
        else
          mem.count_r_P++; */
        
        if(buffer[i].reads>=COUNTERS_SIZE)
          rotate_buffer(&buffer, op);
        break;

      case 'I':
        buffer[i].ifetch++;
        I_count++;

/*         if(buffer[i].m_type=='D')
          mem.count_r_D++;
        else
          mem.count_r_P++; */
        
        if(buffer[i].ifetch>=COUNTERS_SIZE)
          rotate_buffer(&buffer, op);
        break;

      case 'W':
        buffer[i].writes++;
        W_count++;

/*         if(buffer[i].m_type=='D')
          mem.count_w_D++;
        else
          mem.count_w_P++; */
        
        if(buffer[i].writes>=COUNTERS_SIZE)
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


      if (memory_accesses==TIME_TO_MIGRATE) //seleciona a politica e efetiva a migração
      {
        switch (M_POLICY)
        {
        case 'a'://always
          rec = m.always_migrate(&buffer);
          break;
        case 'f'://fuzzy
          rec = m.fuzzy_recommendation(&buffer, COUNTERS_SIZE, BUFFER_SIZE, DEBBUG);
          break;
        case 'c'://coin
          rec = m.coin_recommendation(buffer[i], op);
          break;
        case 'e'://oracle's eye
          predict.push_back(buffer);
          buffer.clear();
          break;
        case 'o'://oracle
          rec = m.oracle_predictor(oracle_memory_in,&mem);
          break;
        default:
          break;
        }
        
        if(DEBBUG)
        {
          //cout << "\nRecomenda migrar:\n";
          //print_buffer_v(&rec);
          cout << "Buffer:\n";
          print_buffer_v(&buffer);
          //getchar();

        }

        m.migrate_mem(rec, &mem); //faz a migração na memoria hibrida
        //if(M_POLICY!='e' && M_POLICY!='o')
        m.migrate_buffer(rec, &buffer); //atualiza a migração no buffer
        memory_accesses=0;
      }
      if(DEBBUG)
        {
          //cout<< "===========  " << op << " " << buffer[i].addr << "  ===========\n\n";
          //cout << "Buffer:\n";
          //print_buffer_v(&buffer);
          //cout << "Mem:\n";
          //mem.print();
          //mem.print_stats();
          //getchar();
          //system("clear");
        }
  }
  
  fclose(arq);

  if(M_POLICY=='e')//oracle's eye
  {
      ofstream oracle_memory_out("oracle_memory.dat");
      Page max_read, max_write;
      int diff_read, diff_write;
      diff_read=0;
      diff_write=0;
      max_read.addr=-1;
      max_read.addr=-1;
      for(int i = 0 ; i<predict.size(); i++)
      {
        for (int j = 0; j < predict[i].size(); j++)
        {
          if(predict[i][j].reads - predict[i][j].writes > diff_read)
          {
              max_read.addr=predict[i][j].addr;
              diff_read = predict[i][j].reads - predict[i][j].writes;
          }
          if(predict[i][j].writes - predict[i][j].reads > diff_write)
          {
              max_write.addr=predict[i][j].addr;
              diff_write = predict[i][j].writes - predict[i][j].reads;
          }
        }
        oracle_memory_out << max_read.addr << ";" << diff_read << ";" << max_write.addr << ";" << diff_write <<"\n";
        diff_read=0;
        diff_write=0;
        max_read.addr=-1;
        max_read.addr=-1;
      }
      oracle_memory_out.close();
  }
  mem.print_stats();
  //mem.print_stats_clean();
  //imprime parametros da simulacao
  cout <<  "BUFFER_SIZE"  << ";" << "TIME_TO_MIGRATE" << ";" << "COUNTERS_SIZE" << ";" << "PROMOTE_VALUE" << ";" << "DEMOTE_VALUE" << '\n';
  cout <<  BUFFER_SIZE  << ";" << TIME_TO_MIGRATE << ";" << COUNTERS_SIZE << ";" << PROMOTE_VALUE << ";" << DEMOTE_VALUE << '\n';
}

