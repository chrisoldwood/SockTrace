/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		CONNSDLG.HPP
** COMPONENT:	The Application
** DESCRIPTION:	The CConnsDlg class declaration.
**
*******************************************************************************
*/

// Check for previous inclusion
#ifndef CONNSDLG_HPP
#define CONNSDLG_HPP

/******************************************************************************
** 
** .
**
*******************************************************************************
*/

class CConnsDlg : public CDialog
{
public:
	//
	// Constructors/Destructor.
	//
	CConnsDlg();
	
protected:
	//
	// Members.
	//
	
	//
	// Controls.
	//
	CListView	m_lvSocks;

	// Columns.
	enum
	{
		INSTANCE,
		STATUS,
		PROTOCOL,
		HOST,
		PORT,
		SENT,
		RECV,
	};

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

#endif // CONNSDLG_HPP
