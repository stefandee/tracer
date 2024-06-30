//---------------------------------------------------------------------------
#ifndef t_mainH
#define t_mainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:	// IDE-managed Components
        TTimer *Timer;
        TLabel *lbLog;
        TMemo *memoLog;
        TLabel *lTarget;
        TListBox *lbTarget;
        TLabel *lComputerName;
        TLabel *lUserName;
        void __fastcall TimerTimer(TObject *Sender);
        
        void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
        
private:	// User declarations
public:		// User declarations
        __fastcall TMainForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
