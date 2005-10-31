// Copyright (C) 2004 TightVNC Development Team. All Rights Reserved.
//
//  TightVNC is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// TightVNC homepage on the Web: http://www.tightvnc.com/

// AdministrationControls.cpp: implementation of the AdministrationControls class.

#include "AdministrationControls.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AdministrationControls::AdministrationControls(HWND hwnd, vncServer * server)
{
	m_server = server;
	m_hwnd = hwnd;
	Init();
}

void AdministrationControls::Validate()
{
	if (!IsChecked(IDALLOWLOOPBACK))
		SetChecked(IDONLYLOOPBACK, false);
	Enable(IDONLYLOOPBACK, IsChecked(IDALLOWLOOPBACK));
	Enable(IDLOGLOTS, IsChecked(IDLOG));
	Enable(IDC_URL_PARAMS, IsChecked(IDENABLEHTTPD));
}

void AdministrationControls::Apply()
{
	if (IsChecked(IDPRIORITY1))
		m_server->SetConnectPriority(1);
	if (IsChecked(IDPRIORITY2))
		m_server->SetConnectPriority(2);
	if (IsChecked(IDPRIORITY0))
		m_server->SetConnectPriority(0);

	m_server->SetAuthRequired(IsChecked(IDREQUIREAUTH));
	m_server->SetHttpdEnabled(IsChecked(IDENABLEHTTPD),
							  IsChecked(IDC_URL_PARAMS));
	m_server->SetLoopbackOk(IsChecked(IDALLOWLOOPBACK));
	m_server->SetLoopbackOnly(IsChecked(IDONLYLOOPBACK));

	if (IsChecked(IDLOG))
		vnclog.SetMode(2);
	else
		vnclog.SetMode(0);

	if (IsChecked(IDLOGLOTS))
		vnclog.SetLevel(10);
	else
		vnclog.SetLevel(2);
}

void AdministrationControls::Init()
{
	SetChecked(IDENABLEHTTPD, m_server->HttpdEnabled());
	SetChecked(IDC_URL_PARAMS, m_server->HttpdParamsEnabled());
	SetChecked(IDALLOWLOOPBACK, m_server->LoopbackOk());
	SetChecked(IDONLYLOOPBACK, m_server->LoopbackOnly());
	SetChecked(IDREQUIREAUTH, m_server->AuthRequired());

	int priority = m_server->ConnectPriority();
	
	SetChecked(IDPRIORITY1, (priority == 1));	
	SetChecked(IDPRIORITY2, (priority == 2));	
	SetChecked(IDPRIORITY0, (priority == 0));
	
	SetChecked(IDLOG, (vnclog.GetMode() >= 2));	
	SetChecked(IDLOGLOTS, (vnclog.GetLevel() > 5));
	
	Validate();
}

AdministrationControls::~AdministrationControls()
{

}
