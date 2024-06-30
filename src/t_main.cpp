//---------------------------------------------------------------------------
#include <vcl.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#pragma hdrstop

#include <shellapi.h>
#include "t_main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

#define MaxAppCount 64
#define AppListFile "applist.txt"

typedef struct _AppListEntry {
        AnsiString name;
        SYSTEMTIME startime, endtime;
        BOOL active;
} *pAppListEntry;


TMainForm *MainForm;

char* days[7] = {"duminica", "luni", "martzi", "miercuri", "joi", "vineri", "simbata" };
char* pszComputerName;
char* pszUserName;
char* pszLocalPath;
char* pszRemotePath = "\\\\TDNTSERVER\\Audit";
char* pszWinDir;

int localfile = -1;
int mirrorfile = -1;

pAppListEntry AppList;
int AppCount;

// initializeaza AppList
// param  : none
// return : cod de eroare (-1)
int InitAppList()
{
  int handle, flength, i;
  AnsiString path;
  char* buffer;

  //path = AnsiString(pszLocalPath) + "\\" + AppListFile;
  path = AnsiString(pszWinDir) + "\\" + AppListFile;

  if ((handle = open(path.c_str(), O_BINARY | O_RDONLY)) == -1) return -1;
  flength = filelength(handle);
  if (flength == -1) return -1;
  buffer = new char[flength+1];
  if (!read(handle, buffer, flength)) return -1;
  close(handle);

  AppCount = 0;
  AppList = new struct _AppListEntry[MaxAppCount];

  for(i = 0; i < MaxAppCount; i++) AppList[i].active = false;
  for(i = 0; i < flength+1; i++)
  {                                     
    if (buffer[i] == '\r')
    {
      if (!AppList[AppCount].name.IsEmpty())
      {
        AppCount++;
        if (AppCount > MaxAppCount-1) break;
      }
      i++;
    }
    else AppList[AppCount].name += buffer[i];
  }

  // adaug in lista lbTarget
  for(i = 0; i < AppCount; i++) MainForm->lbTarget->Items->Add(AppList[i].name.c_str());

  delete buffer;
  return 1;
}
//---------------------------------------------------------------------------

char AvailableDrive()
{
  DWORD drives = GetLogicalDrives(), mask = 4;
  int i;

  if (drives == 0) return 0;
  for(i = 2; i<31; i++)
  {
    if ((drives & mask) == 0) return ('A'+i);
    mask = mask << 1;
  }
  return 0;
}

// new line
void nl(int handle)
{
  char newline = '\n';
  if (handle) write(handle, &newline, 1);
}

// tab
void tab(int handle)
{
  char spaces[8] = "        ";
  if (handle) write(handle, &spaces, 8);
}

// write a string
void wr(int handle, char* what)
{
  if (handle) write(handle, what, strlen(what));
}

// write a string and a nl
void wrnl(int handle, char* what)
{
  if (handle) write(handle, what, strlen(what));
  nl(handle);
}


//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
        : TForm(Owner)
{
  char* path = new char[128];
  DWORD buflen = 127;

  pszComputerName = new char[128];
  pszUserName = new char[128];
  pszLocalPath = new char[128];
  pszWinDir = new char[128];

  GetComputerName(pszComputerName, &buflen);
  buflen = 127;
  GetUserName(pszUserName, &buflen);

  lComputerName->Caption = "Computer : " + AnsiString(pszComputerName);
  lUserName->Caption = "User : " + AnsiString(pszUserName);

  if ((pszLocalPath[0] = AvailableDrive()) == -1)
  {
    // nu am un disk logic liber
    Application->Terminate();
  }
  pszLocalPath[1] = ':'; pszLocalPath[2] = '\0';

  // teste cu wnetaddconnection2
  // se deschide conexiunea la resursa din retzea

  /*
  LPNETRESOURCE netres;
  netres = new NETRESOURCE;
  netres->dwType = RESOURCETYPE_DISK;
  netres->lpLocalName = pszLocalPath;
  netres->lpRemoteName = pszRemotePath;
  netres->lpProvider = NULL;

  // !!! de vazut cum merge cu UserName = NULL
  DWORD result = WNetAddConnection2(netres, "lagara", NULL, 0);

  // eroare
  if (result != NO_ERROR)
  {
     // se scoate un mesaj de eroare la debugger
     char* errbuf = new char[255];
     switch (result)
     {
       case ERROR_ACCESS_DENIED    : strcpy(errbuf, "acces denied"); break;
       case ERROR_ALREADY_ASSIGNED	: strcpy(errbuf, "The local device specified by lpLocalName is already connected to a network resource.");break;
       case ERROR_BAD_DEV_TYPE	:strcpy(errbuf, "The type of local device and the type of network resource do not match.");break;
       case ERROR_BAD_DEVICE	:strcpy(errbuf, "The value specified by lpLocalName is invalid.");break;
       case ERROR_BAD_NET_NAME	:strcpy(errbuf, "The value specified by lpRemoteName is not acceptable to any network resource provider. The resource name is invalid, or the named resource cannot be located.");break;
       case ERROR_BAD_PROFILE	:strcpy(errbuf, "The user profile is in an incorrect format.");break;
       case ERROR_BAD_PROVIDER	:strcpy(errbuf, "The value specified by lpProvider does not match any provider.");break;
       case ERROR_BUSY	:strcpy(errbuf, "The router or provider is busy, possibly initializing. The caller should retry.");break;
       case ERROR_CANCELLED	:strcpy(errbuf, "The attempt to make the connection was cancelled by the user through a dialog box from one of the network resource providers, or by a called resource.");break;
       case ERROR_CANNOT_OPEN_PROFILE: strcpy(errbuf, "The system is unable to open the user profile to process persistent connections.");break;
       case ERROR_DEVICE_ALREADY_REMEMBERED :strcpy(errbuf, "An entry for the device specified in lpLocalName is already in the user profile.");break;
       case ERROR_EXTENDED_ERROR	:strcpy(errbuf, "A network-specific error occured. Call the WNetGetLastError function to get a description of the error.");break;
       case ERROR_INVALID_PASSWORD	:strcpy(errbuf, "The specified password is invalid.");break;
       case ERROR_NO_NET_OR_BAD_PATH:strcpy(errbuf, "A network component has not started, or the specified name could not be handled.");break;
       case ERROR_NO_NETWORK       :strcpy(errbuf, "No network");break;
     }
     //MessageBox(0, errbuf, "Error", MB_OK);
     OutputDebugString(errbuf);
     Application->Terminate();
     exit(1);
  }

  // se deschid fisierele
  // intii, fisierul mirror

  strcpy(path, pszRemotePath);
  strcat(path, "\\");
  strcat(path, pszComputerName);
  if ((mirrorfile = open(path, O_RDWR | O_CREAT | O_APPEND, S_IREAD | S_IWRITE)) == -1)
  {
     // eroare critica -> iese
     //MessageBox(0, "Cannot open mirror file", "Error", MB_OK);
     OutputDebugString("Cannot open mirror file");
     WNetCancelConnection(pszLocalPath, 0);
     Application->Terminate();
     exit(1);
  }
  wrnl(mirrorfile, pszComputerName);
  */
  
  // apoi, fisierul local
  if (GetWindowsDirectory(pszWinDir, 128) == 0)
  {
     // non fatal
     OutputDebugString("Cannot get windows directory");
  }
  else
  {
    strcpy(path, pszWinDir);
    strcat(path, "\\");
    strcat(path, pszComputerName);
    if ((localfile = open(path, O_RDWR | O_CREAT | O_APPEND, S_IREAD | S_IWRITE)) == -1)
    {
       // nu e eroare critica
       //MessageBox(0, "Cannot open local file", "Error", MB_OK);
       OutputDebugString("Cannot open local file");
    }
  }
  wrnl(localfile, pszComputerName);

  delete path;

  // se initializeaza AppList
  // se citeste din fisierul de pe server
  if (!InitAppList())
  {
     // eroare critica - nu poate initializa AppList
     //MessageBox(0, "Cannot init AppList", "Fatal error", MB_OK);
     OutputDebugString("Cannot read from applist.txt file");
     if (mirrorfile) close(mirrorfile);
     WNetCancelConnection(pszLocalPath, 0);
     Application->Terminate();
     exit(1);
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::TimerTimer(TObject *Sender)
{
  for(int i = 0; i < AppCount; i++)
  {
    if (FindWindowEx(NULL, NULL, NULL, AppList[i].name.c_str()))
    {
      /* se face test de activ pt ca Tracer ar putea sa fie
         pornit dupa aplicatie */
      if (!AppList[i].active)
      {
         AppList[i].active = TRUE;
         LPSYSTEMTIME systemtime;
         systemtime = new _SYSTEMTIME;
         GetLocalTime(systemtime);
         AppList[i].startime = *systemtime;
      }                    
    }
    else
    {
      /* aplicatia nu e activa
         verific daca nu s'a terminat kiar acum */
      if (AppList[i].active)
      {
         AppList[i].active = FALSE;
         LPSYSTEMTIME systemtime;
         systemtime = new _SYSTEMTIME;
         GetLocalTime(systemtime);
         AppList[i].endtime = *systemtime;
         /* acum baga in memoLog */
         AnsiString temp =
           AnsiString(days[AppList[i].startime.wDayOfWeek]) + "," +
           AnsiString(AppList[i].startime.wDay) + "-" +
           AnsiString(AppList[i].startime.wMonth) + "-" +
           AnsiString(AppList[i].startime.wYear) + "  " +
           AppList[i].name+ " a rulat de la " +
           AnsiString(AppList[i].startime.wHour) + ":" + AnsiString(AppList[i].startime.wMinute) + ":" + AnsiString(AppList[i].startime.wSecond) + " pina la " +
           AnsiString(AppList[i].startime.wHour) + ":" + AnsiString(AppList[i].endtime.wMinute) + ":" + AnsiString(AppList[i].endtime.wSecond);
         memoLog->Lines->Add(temp + "\n");
         wrnl(localfile, temp.c_str());
         wrnl(mirrorfile, temp.c_str());
      }
    }
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormCloseQuery(TObject *Sender, bool &CanClose)
{
  if (mirrorfile) close(mirrorfile);
  if (localfile) close(localfile);
  WNetCancelConnection(pszLocalPath, 0);
}
//---------------------------------------------------------------------------


