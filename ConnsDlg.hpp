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

#if _MSC_VER > 1000
#pragma once
#endif

#include <WCL/CommonUI.hpp>

/******************************************************************************
** 
** The dialog used to display the current connections.
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
