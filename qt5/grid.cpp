// 8 june 2016
#include "uipriv_qt5.hpp"
#include <QGridLayout>
#include <QWidget>

struct uiGrid : public uiQt5Control {};

void uiGridAppend(uiGrid *g, uiControl *c, int left, int top, int xspan, int yspan, int hexpand, uiAlign halign, int vexpand, uiAlign valign)
{
	qWarning("TODO params");

	if (auto layoutGrid = uiValidateAndCastObjTo<QGridLayout>(g)) {
		auto obj = uiValidateAndCastObjTo<QObject>(c);

		Qt::Alignment align ;

		if (auto layout = qobject_cast<QLayout*>(obj)) {
			layoutGrid->addLayout(layout, top, left, xspan, yspan);
		} else if (auto widget = qobject_cast<QWidget*>(obj)) {
			layoutGrid->addWidget(widget, top, left, xspan, yspan);
		} else {
			qWarning("object is neither layout nor widget");
		}
	}
}

void uiGridInsertAt(uiGrid *g, uiControl *c, uiControl *existing, uiAt at, int xspan, int yspan, int hexpand, uiAlign halign, int vexpand, uiAlign valign)
{
	qWarning("TODO");
	// struct gridChild gc;
	// GtkWidget *widget;

	// widget = prepare(&gc, c, hexpand, halign, vexpand, valign);
	// uiControlSetParent(gc.c, uiControl(g));
	// TODO_MASSIVE_HACK(uiUnixControl(gc.c));
	// gtk_grid_attach_next_to(g->grid, widget,
	// 	GTK_WIDGET(uiControlHandle(existing)), gtkPositions[at],
	// 	xspan, yspan);
	// g_array_append_val(g->children, gc);
}

int uiGridPadded(uiGrid *g)
{
	qWarning("TODO");
	return 0;
}

void uiGridSetPadded(uiGrid *g, int padded)
{
	qWarning("TODO");
}

uiGrid *uiNewGrid(void)
{
    QGridLayout *layout = new QGridLayout();

    return uiAllocQt5ControlType(uiGrid, layout, uiQt5Control::DeleteControlOnQObjectFree);
}
