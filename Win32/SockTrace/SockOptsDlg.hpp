/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		SOCKOPTSDLG.HPP
** COMPONENT:	The Application
** DESCRIPTION:	The CSockOptsDlg class declaration.
**
*******************************************************************************
*/

// Check for previous inclusion
#ifndef SOCKOPTSDLG_HPP
#define SOCKOPTSDLG_HPP

/******************************************************************************
** 
** .
**
*******************************************************************************
*/

class CSockOptsDlg : public CDialog
{
public:
	//
	// Constructors/Destructor.
	//
	CSockOptsDlg();
	
protected:
	//
	// Members.
	//
	
	//
	// Controls.
	//

	//
	// Message handlers.
	//
	virtual void OnInitDialog();
	virtual bool OnOk();
};

/******************************************************************************
**
** Implementation of inline functions.
**
*******************************************************************************
*/

#endif // SOCKOPTSDLG_HPP
