#include <gtkmm.h>
#include <string>
#include <curl/curl.h>
#include <vector>
#include <iostream>

//Various variables, global for the sake of simplicity.
GtkBuilder *builder;
GtkWindow *window;
GtkSearchEntry *searchBar;
GtkButton *searchBtn;
GtkLabel *resultLabels[5];
GError *error = NULL;
std::string webpage;

size_t writeCallback(char *buf, size_t size, size_t nmemb, void* up){
  //Buf is the pointer to the data.
  //size*nmemb is the size of the buffer.
  /*for (int i = 0; i < size*nmemb; i++){
    webpage.push_back(buf[i]);
  }*/
  ((std::string*)up)->append((char*)buf, size * nmemb);
  return size*nmemb;
}

static void getRes(){
  //Get search entry:
  std::string searchSubject = std::string(gtk_entry_get_text(GTK_ENTRY(searchBar)));
  std::cout << "Search pokemon: " << searchSubject << std::endl;
  //Put entry into a string:
  std::string searchString = "https://bulbapedia.bulbagarden.net/wiki/";
  searchString.append(searchSubject);
  searchString.append("_(Pok%C3%A9mon)");
  std::cout << searchString << std::endl;

  //Do the curl-a-whirl:
  CURL *curl;
  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, searchString.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &webpage);
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);//For debugging.
  curl_easy_perform(curl);
  //Curl cleanup:
  curl_easy_cleanup(curl);
  curl_global_cleanup();
  //Report on curl request:
  std::cout << "curl request complete" << std::endl;
  
  //Put stat locations into a vector:
  std::vector<size_t> statPos;
  std::string statClue = "width: 30px;\">";
  size_t pos = webpage.find(statClue,0);
  while (pos != std::string::npos){
    statPos.push_back(pos);
    pos = webpage.find(statClue,pos+1);
  }
  std::cout << "Locations pushed to vector" << std::endl;

  //Exit if stats not found:
  if (statPos.size() < 6){
    std::cout << "No stats found" << std::endl;
    return;
  }

  //Find out how many sets of stats:
  int statSets = (statPos.size()/7)-1;
  int startPoint = (statSets*7);
  
  //Put the stats into a vector:
  std::vector<std::string> stats;
  for (int i = startPoint; i < (startPoint+6); i++){
    int statRealPos = statPos[i] + statClue.length();//Get true location.
    std::cout << "Stat: " << i << " is at: " << statRealPos << std::endl;
    std::string statNum = "";
    while ((webpage.compare(statRealPos,1,"<") != 0)){//Loop through webpage till end of stat location.
      if ((std::isdigit(webpage[statRealPos])) != 0){
	statNum += webpage[statRealPos];//Add character to the stat.
      }
      statRealPos++;
    }
    stats.push_back(statNum);
    std::cout << "Stat: " << i << " is: " << statNum << std::endl;
  }
  std::cout << "Stats put in vector" << std::endl;

  //Put stats into labels:
  for (int j = 0; j < 6; j++){
    gtk_label_set_text(resultLabels[j],stats[j].c_str());
  }

  //Flush various data:
  webpage = "";
  statPos.clear();
}

int main(int argc, char *argv[]){
  
  //Init gtk:
  gtk_init(&argc,&argv);
  
  //Construct GtkBuilder instance:
  builder = gtk_builder_new();
  if (gtk_builder_add_from_file(builder,"layout.ui",&error) == 0){
    g_printerr("Error loading file: %s\n", error->message);
    g_clear_error(&error);
    return 1;
  }
  
  //Get GObjects
  window = GTK_WINDOW(gtk_builder_get_object(builder,"window"));
  searchBar = GTK_SEARCH_ENTRY(gtk_builder_get_object(builder,"srhPokemon"));
  searchBtn = GTK_BUTTON(gtk_builder_get_object(builder,"btnGo"));
  resultLabels[0] = GTK_LABEL(gtk_builder_get_object(builder,"lblHPRes"));
  resultLabels[1] = GTK_LABEL(gtk_builder_get_object(builder,"lblAtkRes"));
  resultLabels[2] = GTK_LABEL(gtk_builder_get_object(builder,"lblDefRes"));
  resultLabels[3] = GTK_LABEL(gtk_builder_get_object(builder,"lblSpAtkRes"));
  resultLabels[4] = GTK_LABEL(gtk_builder_get_object(builder,"lblSpDefRes"));
  resultLabels[5] = GTK_LABEL(gtk_builder_get_object(builder,"lblSpdRes"));
   
  //Close gtk if window's destroyed:
  g_signal_connect(window,"destroy",G_CALLBACK(gtk_main_quit), NULL);

  //Attach search function to button click:
  g_signal_connect(searchBtn,"clicked",G_CALLBACK(getRes),NULL);
  gtk_main();
  
  return 0;
}
