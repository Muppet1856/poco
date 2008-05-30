//
// TableRenderer.cpp
//
// $Id: //poco/Main/WebWidgets/ExtJS/src/TableRenderer.cpp#4 $
//
// Library: ExtJS
// Package: Core
// Module:  TableRenderer
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


#include "Poco/WebWidgets/ExtJS/TableRenderer.h"
#include "Poco/WebWidgets/ExtJS/FormRenderer.h"
#include "Poco/WebWidgets/ExtJS/Utility.h"
#include "Poco/WebWidgets/ExtJS/TableCellHandlerFactory.h"
#include "Poco/WebWidgets/ExtJS/ArrayTableSerializer.h"
#include "Poco/WebWidgets/Table.h"
#include "Poco/WebWidgets/WebApplication.h"
#include "Poco/WebWidgets/RequestHandler.h"
#include "Poco/WebWidgets/DateFormatter.h"
#include "Poco/Delegate.h"
#include <sstream>


namespace Poco {
namespace WebWidgets {
namespace ExtJS {


const std::string TableRenderer::EV_CELLCLICKED("cellclick");
const std::string TableRenderer::EV_AFTEREDIT("afteredit");
const std::string TableRenderer::HIDDEN_INDEX_ROW("hidIdx");


TableRenderer::TableRenderer()
{
}


TableRenderer::~TableRenderer()
{
}


void TableRenderer::renderHead(const Renderable* pRenderable, const RenderContext& context, std::ostream& ostr)
{
	poco_assert_dbg (pRenderable != 0);
	poco_assert_dbg (pRenderable->type() == typeid(Poco::WebWidgets::Table));
	const Table* pTable = static_cast<const Poco::WebWidgets::Table*>(pRenderable);

	ostr << "new Ext.grid.EditorGridPanel({";

	TableRenderer::renderProperties(pTable, context, ostr);

	ostr << "})";
}


void TableRenderer::renderBody(const Renderable* pRenderable, const RenderContext& context, std::ostream& ostr)
{
}


void TableRenderer::addCellValueChangedServerCallback(Table* pTable, const std::string& onSuccess, const std::string& onFailure)
{
	poco_check_ptr (pTable);
	static const std::string signature("function(obj)");
	//extract the true row index from the last column!
	std::string origRow("+obj.record.get('");
	origRow.append(Poco::NumberFormatter::format(static_cast<Poco::UInt32>(pTable->getColumnCount())));
	origRow.append("')");
	std::map<std::string, std::string> addParams;
	addParams.insert(std::make_pair(Table::FIELD_COL, "+obj.column"));
	addParams.insert(std::make_pair(Table::FIELD_ROW, origRow));
	//problem: I need the displayed string from teh renderer, not the value!
	// date fields cause problems here, and I only habe one cellclick event per table not per column!
	// from the tabel get the TableCOlumn, from thsi get the renderer for the given col and render obj.value
	// {(var r=obj.grid.getColumnModel().getRenderer(obj.col))?r(obj.value);:obj.value;} hm, no working
	addParams.insert(std::make_pair(Table::FIELD_VAL, "+obj.value")); 
	addParams.insert(std::make_pair(RequestHandler::KEY_EVID, Table::EV_CELLVALUECHANGED));
	Utility::addServerCallback(pTable->cellValueChanged, signature, addParams, pTable->id(), onSuccess, onFailure);
	pTable->cellValueChanged.add(jsDelegate("function(obj){obj.grid.getStore().commitChanges();}"));
}




void TableRenderer::addCellClickedServerCallback(Table* pTable, const std::string& onSuccess, const std::string& onFailure)
{
	poco_check_ptr (pTable);
	static const std::string signature("function(theGrid,row,col,e)");
	//extract the true row index from the last column!
	std::string origRow("+theGrid.getStore().getAt(row).get('");
	origRow.append(Poco::NumberFormatter::format(static_cast<Poco::UInt32>(pTable->getColumnCount())));
	origRow.append("')");
	std::map<std::string, std::string> addParams;
	addParams.insert(std::make_pair(Table::FIELD_COL, "+col"));
	addParams.insert(std::make_pair(Table::FIELD_ROW, origRow));
	addParams.insert(std::make_pair(RequestHandler::KEY_EVID, Table::EV_CELLCLICKED));
	Utility::addServerCallback(pTable->cellClicked, signature, addParams, pTable->id(), onSuccess, onFailure);
}


void TableRenderer::renderProperties(const Table* pTable, const RenderContext& context, std::ostream& ostr)
{
	WebApplication& app = WebApplication::instance();
	Renderable::ID id = pTable->id();
	Utility::writeRenderableProperties(pTable, ostr);

	ostr << ",listeners:{";
	bool written = Utility::writeJSEvent(ostr, EV_AFTEREDIT, pTable->cellValueChanged.jsDelegates());
	if (written)
		ostr << ",";
	
	
	Utility::writeJSEvent(ostr, EV_CELLCLICKED, pTable->cellClicked.jsDelegates());
	
	ostr << "},"; //close listeners
	
	renderColumns(pTable, context, ostr);
	// forbid reordering of columns, otherwise col index will not match the col index at the server
	// sorting is allowed though, i.e row matching is active
	ostr << ",clicksToEdit:1,stripeRows:true,enableColumnHide:false,enableColumnMove:false";
	if (pTable->getWidth() > 0)
		ostr << ",width:" << pTable->getWidth();
	if (pTable->getHeight() > 0)
		ostr << ",height:" << pTable->getHeight();
	ostr << ",store:";
	renderStore(pTable, ostr);
	Table* pT = const_cast<Table*>(pTable);
	pT->beforeLoad += Poco::delegate(&TableRenderer::onBeforeLoad);
	WebApplication::instance().registerAjaxProcessor(Poco::NumberFormatter::format(id), pT);
}


void TableRenderer::renderColumns(const Table* pTable, const RenderContext& context, std::ostream& ostr)
{
	
	//columns: [...]
	ostr << "columns:[";
	const Table::TableColumns& columns = pTable->getColumns();
	Table::TableColumns::const_iterator it = columns.begin();
	int i = 0;
	for (; it != columns.end(); ++it, ++i)
	{
		if (i != 0)
			ostr << ",";

		renderColumn(pTable, *(*it), i, context, ostr);
	}
	// must be last column, so we don't need row/col correction!
	// the last column is required to be able to handle sorting on the client side and to 
	// match client-row indizes to server-row indizes
	// we use the very first entry as dummy data index
	ostr << ",{id:'" << HIDDEN_INDEX_ROW << "',header:'" << HIDDEN_INDEX_ROW << "',dataIndex:'" << i << "',";
	ostr << "hidden:true}";
	ostr << "]";
}


void TableRenderer::renderColumn(const Table* pTable, const TableColumn& tc, int idx, const RenderContext& context, std::ostream& ostr)
{
	static LookAndFeel& laf = Utility::getDefaultRenderers();

	// {id:'company', header: "Company", width: 200, sortable: true, dataIndex: 'company'}
	// {header: "Last Updated", width: 135, sortable: true, renderer: Ext.util.Format.dateRenderer('m/d/Y'), dataIndex: 'lastChange'}
	ostr << "{";
	std::string hdr(Utility::safe(tc.getHeader()));

	ostr << "header:'" << hdr << "',dataIndex:'" << idx << "'";

	if (tc.getWidth() > 0)
		ostr << ",width:" << tc.getWidth();
	if (tc.isSortable())
		ostr << ",sortable:true";
	
	static TableCellHandlerFactory& fty = TableCellHandlerFactory::instance();

	if (tc.getCell())
	{
		AbstractTableCellHandler::Ptr pHandler = fty.factory(tc.getCell());
		if (tc.getCell()->isEditable())
			ostr << ",editable:true";
		if (tc.getCell()->isEditable() && pHandler->useEditor())
		{
			ostr << ",editor:";
			tc.getCell()->renderHead(context, ostr);
		}
	
		if (pHandler->useRenderer())
		{
			ostr << ",renderer:";
			pHandler->writeDynamicData(ostr);
		}
	}

	ostr << "}";
}


void TableRenderer::renderStore(const Table* pTable, std::ostream& ostr)
{

	//new Ext.data.SimpleStore({
	//	fields: [
	//	   {name: 'company'},
	//	   {name: 'price', type: 'float'},
	//	   {name: 'change', type: 'float'},
	//	   {name: 'pctChange', type: 'float'},
	//	   {name: 'lastChange', type: 'date', dateFormat: 'n/j h:ia'}
	//	],
	//  proxy: new Ext.data.HttpProxy({url:'/myuri;...'}),
	//  reader: new Ext.data.ArrayReader()
	//});

	// we don't know the type, we just have a formatter, the name is always the idx!
	// we use the formatter later to set a renderer for a different type than string
	const Table::TableColumns& columns = pTable->getColumns();
	ostr << "new Ext.data.SimpleStore({autoLoad:true,fields:[";
	Table::TableColumns::const_iterator it = columns.begin();
	int i = 0;
	for (; it != columns.end(); ++it, ++i)
	{
		if (i != 0)
			ostr << ",";
		if ((*it)->getCell()->type() == typeid(Poco::WebWidgets::DateFieldCell))
		{
			Poco::WebWidgets::DateFieldCell::Ptr pDf = (*it)->getCell().cast<Poco::WebWidgets::DateFieldCell>();
			
			ostr << "{name:'" << i << "',type:'date',dateFormat:'" << Utility::convertPocoDateToPHPDate(pDf->getFormat()) << "'}";
		}
		else
			ostr << "{name:'" << i << "'}";
	}
	ostr << ",{name:'" << i << "'}";
	ostr << "],"; // close fields
	ostr << "proxy:new Ext.data.HttpProxy({url:";
	std::map<std::string, std::string> addParams;
	addParams.insert(std::make_pair(RequestHandler::KEY_EVID,Table::EV_LOADDATA));
	
	std::string url(Utility::createURI(addParams, pTable->id()));
	ostr << url << "}),";
	ostr << "reader:new Ext.data.ArrayReader()";
	//Write data
	/*ostr << "data:";
	renderDataModel(pTable, ostr);*/
	ostr << "})";
}


void TableRenderer::onBeforeLoad(void* pSender, Table::LoadData& ld)
{
	ld.pResponse->setChunkedTransferEncoding(true);
	ld.pResponse->setContentType(ArrayTableSerializer::contentType());
	std::ostream& out = ld.pResponse->send();
	ArrayTableSerializer::serialize(out, ld.pTable, ld.firstRow, ld.rowCnt);
}


} } } // namespace Poco::WebWidgets::ExtJS
