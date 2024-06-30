//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USERES("Tracer.res");
USEFORM("t_main.cpp", MainForm);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "Tracer";
                 Application->CreateForm(__classid(TMainForm), &MainForm);
                 // initializeaza modul stealth :-)
                 // aplicatia e vizibila numai in task menu
                 // Application->ShowMainForm = false;
                 Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;
}
//---------------------------------------------------------------------------
