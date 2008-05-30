//
// Panel.cpp
//
// $Id: //poco/Main/WebWidgets/src/Panel.cpp#7 $
//
// Library: WebWidgets
// Package: Views
// Module:  Panel
//
// Copyright (c) 2007, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#include "Poco/WebWidgets/Panel.h"


namespace Poco {
namespace WebWidgets {


Panel::Panel():
	View(typeid(Panel)),
	_pChild(),
	_title(),
	_modal(false),
	_showCloseIcon(true)
{
}


Panel::Panel(const std::string& name):
	View(name, typeid(Panel)),
	_pChild(),
	_title(),
	_modal(false),
	_showCloseIcon(true)
{
}


Panel::Panel(const std::string& name, const std::string& title):
	View(name, typeid(Panel)),
	_pChild(),
	_title(title),
	_modal(false),
	_showCloseIcon(true)
{
}


Panel::Panel(View::Ptr pChild):
	View(typeid(Panel)),
	_pChild(),
	_title(),
	_modal(false),
	_showCloseIcon(true)
{
	setChild(pChild);
}


Panel::Panel(const std::string& name, View::Ptr pChild):
	View(name, typeid(Panel)),
	_pChild(),
	_title(),
	_modal(false),
	_showCloseIcon(true)
{
	setChild(pChild);
}


Panel::Panel(const std::string& name, const std::string& title, View::Ptr pChild):
	View(name, typeid(Panel)),
	_pChild(),
	_title(title),
	_modal(false),
	_showCloseIcon(true)
{
	setChild(pChild);
}


Panel::~Panel()
{
}


void Panel::setChild(View::Ptr pChild)
{
	if (_pChild)
		rejectChild(_pChild);
	if (pChild)
		adoptChild(pChild);
	_pChild = pChild;
}


View::Ptr Panel::findChild(const std::string& name) const
{
	if (_pChild && name == _pChild->getName())
		return _pChild;

	return View::Ptr();
}


} } // namespace Poco::WebWidgets
