/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		SOCKOPTSDLG.CPP
** COMPONENT:	The Application
** DESCRIPTION:	CSockOptsDlg class definition.
**
*******************************************************************************
*/

#include "AppHeaders.hpp"
#include "SockOptsDlg.hpp"

#ifdef _DEBUG
// For memory leak detection.
#define new DBGCRT_NEW
#endif

/******************************************************************************
** Method:		Default constructor.
**
** Description:	.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

CSockOptsDlg::CSockOptsDlg()
	: CDialog(IDD_SOCK_OPTIONS)
{
	DEFINE_CTRL_TABLE
	END_CTRL_TABLE

	DEFINE_CTRLMSG_TABLE
	END_CTRLMSG_TABLE
}

/******************************************************************************
** Method:		OnInitDialog()
**
** Description:	Initialise the dialog.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CSockOptsDlg::OnInitDialog()
{
}

/******************************************************************************
** Method:		OnOk()
**
** Description:	The OK button was pressed.
**
** Parameters:	None.
**
** Returns:		true or false.
**
*******************************************************************************
*/

bool CSockOptsDlg::OnOk()
{
	return true;
}
