#include "uipriv_qt5.hpp"

#include <QRadioButton>
#include <QVBoxLayout>

struct uiRadioButtons : public uiQt5Control {};


int uiRadioButtonsSelected(uiRadioButtons *r)
{
	if (auto layout = uiValidateAndCastObjTo<QLayout>(r)) {
		for(int i = 0; i < layout->count(); i++){
			if(((QRadioButton*) layout->itemAt(i)->widget())->isChecked()){
				return i;
			}
		}
	}
	return -1;
}

void uiRadioButtonsSetSelected(uiRadioButtons *r, int n)
{
	if (auto layout = uiValidateAndCastObjTo<QLayout>(r)) {
		for(int i = 0; i < layout->count(); i++){
			QRadioButton *btn = (QRadioButton*) layout->itemAt(i)->widget();
			btn->setChecked(i==n);
		}
	}
}

void uiRadioButtonsOnSelected(uiRadioButtons *r, void (*f)(uiRadioButtons *, void *), void *data)
{
	qWarning("TODO uiRadioButtonsOnSelected");
}

void uiRadioButtonsAppend(uiRadioButtons *r, const char *text)
{
	if (auto layout = uiValidateAndCastObjTo<QLayout>(r)) {
		auto radioButton = new QRadioButton(QString::fromUtf8(text));
		layout->addWidget(radioButton);
		if (layout->count() == 1) {
			radioButton->setChecked(true);
		}
	}
}

uiRadioButtons *uiNewRadioButtons(void)
{
	// TODO: check does this need a QButtonGroup or is the layout sufficent?
	auto layout = new QVBoxLayout;
	layout->setSpacing(5);

	// note styling is being set in main.cpp -> styleSheet

	return uiAllocQt5ControlType(uiRadioButtons,layout,uiQt5Control::DeleteControlOnQObjectFree);
}
