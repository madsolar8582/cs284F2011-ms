#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <cmath>

using namespace std;

bool checkPList(string programListFile); //Validate program list file
bool checkPTrace(string programTraceFile); //Validate program trace file
bool checkPSize(long int pSize); //Validate page size
bool checkPAlgorithm(string pageAlgorithm); //Validate replacement algorithm
bool checkPStyle(string pageStyle); //Validate paging style
void checkArguments(int argc, char *argv[]); //Validate all arguments

int main(int argc, char *argv[])
{
  checkArguments(argc, argv);
  
  const int MAX_SIZE = 10; //Number of programs determined by the assignment
  const int memSize = 512; //Size of Memory
  long int pageSize = strtol(argv[3],NULL,10); //Page size determined by the user
  string programListFile = argv[1]; //List File determined by the user
  string programTraceFile = argv[2]; //Trace file determined by the user
  string pageAlgorithm = argv[4]; //Replacement algorithm determined by the user
  string pageStyle = argv[5]; //Paging style determined by the user
  vector<int> systemPageTable[MAX_SIZE]; //Page Table used by the simulation (virtual memory)
  vector<int> timeTracker; //Clock/LRU Tracker, adjusted depending on the algorithm chosen
  vector<int> systemMemory; //Memory used by the simulation
  int numPageFaults = 0; //The number of page faults
  int tracker; //Tracks the age of a page
  int clockTracker; //Tracks the clock bit
  bool foundInMemory = false; //Flag that tracks if a page is in memory
  int pageUniqueID; //Unique ID of a page
  int parseProgramID; //Program being read in
  int parseProgramSize; //Program size being read in
  int numPages; //Number of pages assigned to each program
  int memAllocated; //Memory allocated to each program
  int activeProgram; //Current program
  int requestedLocation; //Requested page location
  int pageLocation; //The actual page location
  int oldPage; //Page being examined
  
  ifstream fileIn; //open the list file and begin parsing
  fileIn.open(programListFile.c_str());
  if(fileIn.fail())
  {
    cerr << "The file you wanted has disappeared!" << endl;
    cerr << "Exiting due to catastrophic failure..." << endl;
    fileIn.close();
    exit(1);
  }
  else
  {
    pageUniqueID = 0;
    while(!fileIn.eof())
    {
      fileIn >> parseProgramID;
      fileIn >> parseProgramSize;
      numPages = static_cast<int>(ceil(static_cast<double>(parseProgramSize) / static_cast<double>(pageSize)));
      for(int i = 0; i < numPages; i++)
      {
        systemPageTable[parseProgramID].push_back(pageUniqueID); //assign each page a unique ID
        pageUniqueID++;
      }
    }
  }
  fileIn.close();
  
  memAllocated = static_cast<int>(ceil(static_cast<double>(memSize) / static_cast<double>(pageSize))); //determine the amount of memory for each program
  
  if(pageAlgorithm == "lru" || pageAlgorithm == "fifo")
  {
    for(int i = 0; i < memAllocated; i++)
    {
      timeTracker.push_back(0); //initialize the LRU page age tracker
    }
    for(int i = 0; i < MAX_SIZE; i++)
    {
      for(int j = 0; j < static_cast<int>(floor(static_cast<double>(memAllocated) / static_cast<double>(MAX_SIZE))); j++) //floor
      {
        systemMemory.push_back(systemPageTable[i][j]); //initialize the clock bit tracker
      }
    }
  }
  else
  {
    for(int i = 0; i < memAllocated; i++)
    {
      timeTracker.push_back(1);
    }
    for(int i = 0; i < MAX_SIZE; i++)
    {
      for(int j = 0; j < static_cast<int>(floor(static_cast<double>(memAllocated) / static_cast<double>(MAX_SIZE))); j++) //floor
      {
        systemMemory.push_back(systemPageTable[i][j]); //initial load of memory
      }
    }
  }
  
  if(pageAlgorithm == "lru")
  {
    fileIn.open(programTraceFile.c_str()); //open the trace file and begin parsing
    if(fileIn.fail())
    {
      cerr << "The file you wanted has disappeared!" << endl;
      cerr << "Exiting due to catastrophic failure..." << endl;
      fileIn.close();
      exit(1);
    }
    else
    {
      fileIn >> activeProgram;
      fileIn >> requestedLocation;
      pageLocation = static_cast<int>(ceil(static_cast<double>(requestedLocation) / pageSize)); //determine the actual location of the page
      int i;
      tracker = 0;
      while(!fileIn.eof())
      {
        foundInMemory = false;
        for(i = 0; i < static_cast<int>(systemMemory.size()); i++)
        {
          if(systemMemory[i] == systemPageTable[activeProgram][pageLocation])
          {
            foundInMemory = true; //if the page is found in memory, no page fault, no replacement
          }
          if(foundInMemory)
          {
            break; //prevent unnecessary loops
          }
        }
        if(!foundInMemory)
        {
          numPageFaults++; //page fault occured
          if(static_cast<int>(systemMemory.size()) < memAllocated)
          {
            systemMemory.push_back(systemPageTable[activeProgram][pageLocation]); //put into memory if there are empty spaces for the active program
          }
          else
          {
            oldPage = 0;
            for(int j = 1; j < static_cast<int>(systemMemory.size()); j++)
            {
              if(timeTracker[j] < timeTracker[oldPage])
              {
                oldPage = j;
              }
            }
            systemMemory[oldPage] = systemPageTable[activeProgram][pageLocation]; //the oldest page is swapped for a new one
            timeTracker[oldPage] = tracker;
            if(pageStyle == "p")
            {
              pageLocation++; oldPage++;
              if(oldPage == static_cast<int>(systemMemory.size()))
              {
                oldPage = 0;
              }
              if(pageLocation == static_cast<int>(systemPageTable[activeProgram].size()))
              {
                pageLocation = 0; //not using global page replacement, so bring in the first page of the active program
              }
              systemMemory[oldPage] = systemPageTable[activeProgram][pageLocation]; //next oldest page is swapped for a new one
              timeTracker[oldPage] = tracker;
            }
          }
        }
        else if(foundInMemory)
        {
          timeTracker[i] = tracker; //update the page age
        }
        tracker++;
        fileIn >> activeProgram;
        fileIn >> requestedLocation;
        pageLocation = static_cast<int>(ceil(static_cast<double>(requestedLocation) / pageSize));
      }
    }
  }
  else if(pageAlgorithm == "fifo")
  {
    fileIn.open(programTraceFile.c_str());
    if(fileIn.fail())
    {
      cerr << "The file you wanted has disappeared!" << endl;
      cerr << "Exiting due to catastrophic failure..." << endl;
      fileIn.close();
      exit(1);
    }
    else
    {
      fileIn >> activeProgram;
      fileIn >> requestedLocation;
      pageLocation = static_cast<int>(ceil(static_cast<double>(requestedLocation) / pageSize)); //determine the actual location of the page
      while(!fileIn.eof())
      {
        foundInMemory = false;
        for(int i = 0; i < static_cast<int>(systemMemory.size()); i++)
        {
          if(systemMemory[i] == systemPageTable[activeProgram][pageLocation])
          {
            foundInMemory = true; //no replacement if the page is found in memory
          }
          if(foundInMemory)
          {
            break; //prevent unnecessary loops
          }
        }
        if(!foundInMemory)
        {
          numPageFaults++; //page fault occured
          if(static_cast<int>(systemMemory.size()) < memAllocated)
          {
            systemMemory.push_back(systemPageTable[activeProgram][pageLocation]); //put page into memory if there is empty space in the active program's memory
          }
          else
          {
            oldPage = 0;
            for(int i = 1; i < static_cast<int>(systemMemory.size()); i++)
            {
              if(timeTracker[i] < timeTracker[oldPage])
              {
                oldPage = i;
              }
            }
            systemMemory[oldPage] = systemPageTable[activeProgram][pageLocation]; //swap occurs and the new page is put in
            timeTracker[oldPage] = tracker;
            if(pageStyle == "p")
            {
              pageLocation++; oldPage++;
              if(oldPage == static_cast<int>(systemMemory.size()))
              {
                oldPage = 0;
              }
              if(pageLocation == static_cast<int>(systemPageTable[activeProgram].size()))
              {
                pageLocation = 0; //prevent global page replacement
              }
              systemMemory[oldPage] = systemPageTable[activeProgram][pageLocation]; //another swap occurs and the new page is put in
              timeTracker[oldPage] = tracker;
            }
          }
        }
        tracker++;
        fileIn >> activeProgram;
        fileIn >> requestedLocation;
        pageLocation = static_cast<int>(ceil(static_cast<double>(requestedLocation) / pageSize));
      }
    }
  }
  else
  {
    fileIn.open(programTraceFile.c_str());
    if(fileIn.fail())
    {
      cerr << "The file you wanted has disappeared!" << endl;
      cerr << "Exiting due to catastrophic failure..." << endl;
      fileIn.close();
      exit(1);
    }
    else
    {
      fileIn >> activeProgram;
      fileIn >> requestedLocation;
      pageLocation = static_cast<int>(ceil(static_cast<double>(requestedLocation) / pageSize)); //determine the actual location of the page
      int i;
      clockTracker = 0;
      while(!fileIn.eof())
      {
        foundInMemory = false;
        for(i = 0; i < static_cast<int>(systemMemory.size()); i++)
        {
          if(systemMemory[i] == systemPageTable[activeProgram][pageLocation])
          {
            foundInMemory = true; //if the page is found in memory, no page fault, no swap occurs
          }
          if(foundInMemory)
          {
            break; //prevent unnecessary loops
          }
        }
        if(!foundInMemory)
        {
          numPageFaults++; //page fault occured
          if(static_cast<int>(systemMemory.size()) < memAllocated)
          {
            systemMemory.push_back(systemPageTable[activeProgram][pageLocation]); //if there is empty space in the active program's memory, put into memory
          }
          else
          {
            while(timeTracker[clockTracker] == 1)
            {
              timeTracker[clockTracker] = 0;
              clockTracker++;
              if(clockTracker == static_cast<int>(timeTracker.size()))
              {
                clockTracker = 0; //prevent seg fault
              }
            }
            systemMemory[clockTracker] = systemPageTable[activeProgram][pageLocation]; //swap occurs for the page with clock bit 0
            timeTracker[clockTracker] = 1;
            if(pageStyle == "p")
            {
              pageLocation++; clockTracker++;
              if(clockTracker == static_cast<int>(systemMemory.size()))
              {
                clockTracker = 0;
              }
              if(pageLocation == static_cast<int>(systemPageTable[activeProgram].size()))
              {
                pageLocation = 0;
              }
              systemMemory[clockTracker] = systemPageTable[activeProgram][pageLocation]; //swaps the next page with clock bit 0
              timeTracker[clockTracker] = 1; 
            }
          }
        }
        else if(foundInMemory)
        {
          timeTracker[i] = 1; //update the clock bit
        }
        fileIn >> activeProgram;
        fileIn >> requestedLocation;
        pageLocation = static_cast<int>(ceil(static_cast<double>(requestedLocation) / pageSize));
      }
    }
  }
  fileIn.close();
  
  cout << "---------------------------------------------------" << endl; //begin outputting the results
  cout << "> Program List File: " << programListFile << endl;
  cout << "> Program Trace File: " << programTraceFile << endl;
  cout << "> Page Size: " << pageSize << endl;
  cout << "> Page Replacement Algorithm: ";
  if(pageAlgorithm == "lru")
  {
    cout << "Least Recently Used" << endl;
  }
  else if(pageAlgorithm == "fifo")
  {
    cout << "First In-First Out" << endl;
  }
  else
  {
    cout << "Clock Based Policy" << endl;
  }
  cout << "> Paging Style: "; 
  if(pageStyle == "p")
  {
    cout << "Prepaging" << endl;
  }
  else
  {
    cout << "Demand" << endl;
  }
  cout << "> Number of Page Faults: " << numPageFaults << endl;
  cout << "---------------------------------------------------" << endl;
    
  return 0;
}

bool checkPList(string programListFile) //check to make sure the file exists
{
  ifstream listFile;
  listFile.open(programListFile.c_str());
  if(listFile.fail())
  {
    cerr << "Unable to open " << programListFile << " !" << endl;
    cerr << "Please make sure the file exists and re-execute the program." << endl;
    listFile.close();
    return false;
  }
  else
  {
    listFile.close();
    return true;
  }
}

bool checkPTrace(string programTraceFile) //check to make sure the file exists
{
  ifstream traceFile;
  traceFile.open(programTraceFile.c_str());
  if(traceFile.fail())
  {
    cerr << "Unable to open " << programTraceFile << " !" << endl;
    cerr << "Please make sure the file exists and re-execute the program." << endl;
    traceFile.close();
    return false;
  }
  else
  {
    traceFile.close();
    return true;
  }
}

bool checkPSize(long int pSize) //check to make sure the user entered a valid page size
{
  bool isValid = false;
  switch (pSize) 
  {
    case 1:
      isValid = true;
      break;
    case 2:
      isValid = true;
      break;
    case 4:
      isValid = true;
      break;
    case 8:
      isValid = true;
      break;
    case 16:
      isValid = true;
      break;
    case 32:
      isValid = true;
      break;
    default:
      cerr << "Invalid page size detected!" << endl;
      cerr << "Valid page sizes are: 1, 2, 4, 8, 16, & 32." << endl;
      cerr << "Please re-execute the program with a valid page size." << endl;
      isValid = false;
  }
  return isValid;
}

bool checkPAlgorithm(string pageAlgorithm) //check to make sure the user entered a valid page replacement algorithm
{
  if(pageAlgorithm == "clock")
  {
    return true;
  }
  else if(pageAlgorithm == "fifo")
  {
    return true;
  }
  else if(pageAlgorithm == "lru")
  {
    return true;
  }
  else
  {
    cerr << "No valid paging algorithm was detected!" << endl;
    cerr << "Valid algorithms are: clock, fifo, & lru." << endl;
    cerr << "Please re-execute the program with a valid paging algorthm." << endl;
    return false;
  }
}

bool checkPStyle(string pageStyle) //check to make sure the user entered a valid paging style
{
  if(pageStyle == "d")
  {
    return true;
  }
  else if(pageStyle == "p")
  {
    return true;
  }
  else
  {
    cerr << "No valid paging style was detected!" << endl;
    cerr << "Valid paging styles are: d or p." << endl;
    cerr << "Please re-execute the program with a valid paging algorithm." << endl;
    return false;
  }
}

void checkArguments(int argc, char *argv[]) //check all the arguments passed into the program
{
  if(argc < 6)
  {
    cerr << "Too few arguments have been passed to the program." << endl;
    cerr << "Arguments should be: programlist programtrace pagesize pagealgorithm pagingstyle" << endl;
    cerr << "Please re-execute the program with the correct number of arguments." << endl;
    exit(1);
  }
  else if(argc > 6)
  {
    cerr << "Too many arguments have been passed to the program." << endl;
    cerr << "Arguments should be: programlist programtrace pagesize pagealgorithm pagingstyle" << endl;
    cerr << "Please re-execute the program with the correct number of arguments." << endl;
    exit(1);
  }
  else if(!checkPList(argv[1]))
  {
    exit(1);
  }
  else if(!checkPTrace(argv[2]))
  {
    exit(1);
  }
  else if(!checkPSize(strtol(argv[3],NULL,10)))
  {
    exit(1);
  }
  else if(!checkPAlgorithm(argv[4]))
  {
    exit(1);
  }
  else if(!checkPStyle(argv[5]))
  {
    exit(1);
  }
  return;
}
