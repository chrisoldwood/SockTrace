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

#if _MSC_VER > 1000
#pragma once
#endif

#include <WCL/CommonUI.hpp>
#include "SockConfig.hpp"

/******************************************************************************
** 
** The dialog used to configure the sockets to trace.
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
	~CSockOptsDlg();
	
	//
	// Members.
	//
	CSockConfigs	m_aoConfigs;
	bool			m_bModified;
	
protected:
	//
	// Controls.
	//
	CListView	m_lvSocks;

	// Columns.
	enum
	{
		SRC_PORT,
		SRC_PROTOCOL,
		DST_HOST,
		DST_PORT,
	};

	//
	// Message handlers.
	//
	virtual void OnInitDialog();
	virtual bool OnOk();
	LRESULT OnListDoubleClick(NMHDR& oHdr);

	void OnAdd();
	void OnEdit();
	void OnRemove();

	//
	// Internal methods.
	//
	CSockConfig* ItemConfig(size_t nItem);
	void         UpdateConfig(size_t nItem, CSockConfig* pConfig);
};

/******************************************************************************
**
** Implementation of inline functions.
**
*******************************************************************************
*/

inline CSockConfig* CSockOptsDlg::ItemConfig(size_t nItem)
{
	ASSERT(nItem < m_lvSocks.ItemCount());

	return static_cast<CSockConfig*>(m_lvSocks.ItemPtr(nItem));
}

#endif // SOCKOPTSDLG_HPP
