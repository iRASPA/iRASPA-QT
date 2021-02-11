INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

include(undocommands.pri)

SOURCES += \
    $$PWD/appearanceprimitiveform.cpp \
    $$PWD/atomtreeviewpushbuttonstyleditemdelegate.cpp \
    $$PWD/bondlistpushbuttonstyleditemdelegate.cpp \
    $$PWD/bondlistviewcomboboxstyleditemdelegate.cpp \
    $$PWD/celltransformcontentform.cpp \
    $$PWD/frameliststyleditemdelegate.cpp \
    $$PWD/framelistviewproxystyle.cpp \
    $$PWD/importfiledialog.cpp \
    $$PWD/iraspatreeview.cpp \
    $$PWD/main.cpp \
    $$PWD/mainwindow.cpp \
    $$PWD/moviemaker.cpp \
    $$PWD/openiraspaformatdialog.cpp \
    $$PWD/projecttreeview.cpp \
    $$PWD/projecttreeviewproxystyle.cpp \
    $$PWD/projecttreeviewstyleditemdelegate.cpp \
    $$PWD/qdoubledial.cpp \
    $$PWD/renderstackedwidget.cpp \
    $$PWD/aboutdialog.cpp \
    $$PWD/iraspatoolbar.cpp \
    $$PWD/mastertoolbar.cpp \
    $$PWD/cameratreewidgetcontroller.cpp \
    $$PWD/savecifformatdialog.cpp \
    $$PWD/saveiraspaformatdialog.cpp \
    $$PWD/savemmcifformatdialog.cpp \
    $$PWD/savemoviedialog.cpp \
    $$PWD/savepdbformatdialog.cpp \
    $$PWD/savepicturedialog.cpp \
    $$PWD/saveposcarformatdialog.cpp \
    $$PWD/savexyzformatdialog.cpp \
    $$PWD/scenetreeview.cpp \
    $$PWD/framelistview.cpp \
    $$PWD/detailtabviewcontroller.cpp \
    $$PWD/cameracameraform.cpp \
    $$PWD/iraspamainwindowconsumerprotocol.cpp \
    $$PWD/atomtreeview.cpp \
    $$PWD/atomtreeviewmodel.cpp \
    $$PWD/framelistviewmodel.cpp \
    $$PWD/scenetreeviewmodel.cpp \
    $$PWD/informationpanelview.cpp \
    $$PWD/scenetreeviewproxystyle.cpp \
    $$PWD/scenetreeviewstyleditemdelegate.cpp \
    $$PWD/textfield.cpp \
    $$PWD/masterstackedwidget.cpp \
    $$PWD/cellcellform.cpp \
    $$PWD/celltreewidgetcontroller.cpp \
    $$PWD/customintspinbox.cpp \
    $$PWD/customdoublespinbox.cpp \
    $$PWD/appearanceatomsform.cpp \
    $$PWD/appearancetreewidgetcontroller.cpp \
    $$PWD/appearancebondsform.cpp \
    $$PWD/appearanceunitcellform.cpp \
    $$PWD/appearanceadsorptionsurfaceform.cpp \
    $$PWD/appearanceannotationform.cpp \
    $$PWD/cameraselectionform.cpp \
    $$PWD/cameralightsform.cpp \
    $$PWD/camerapicturesform.cpp \
    $$PWD/camerabackgroundform.cpp \
    $$PWD/elementsform.cpp \
    $$PWD/infocreatorform.cpp \
    $$PWD/infocreationform.cpp \
    $$PWD/infochemicalform.cpp \
    $$PWD/infocitationform.cpp \
    $$PWD/cellstructuralform.cpp \
    $$PWD/cellsymmetryform.cpp \
    $$PWD/infotreewidgetcontroller.cpp \
    $$PWD/qdoubleslider.cpp \
    $$PWD/selectcolorbutton.cpp \
    $$PWD/fixedaspectratiolayoutitem.cpp \
    $$PWD/bondlistviewmodel.cpp \
    $$PWD/bondlistview.cpp \
    $$PWD/bondlistviewsliderstyleditemdelegate.cpp \
    $$PWD/qtsegmentcontrol.cpp \
    $$PWD/elementlistwidgetcontroller.cpp \
    $$PWD/linenumberplaintextedit.cpp \
    $$PWD/logviewcontroller.cpp \
    $$PWD/pythonviewcontroller.cpp \
    $$PWD/helpbrowser.cpp \
    $$PWD/helpwidget.cpp \
    $$PWD/projecttreeviewmodel.cpp

HEADERS += \
    $$PWD/appearanceprimitiveform.h \
    $$PWD/atomtreeviewpushbuttonstyleditemdelegate.h \
    $$PWD/bondlistpushbuttonstyleditemdelegate.h \
    $$PWD/bondlistviewcomboboxstyleditemdelegate.h \
    $$PWD/celltransformcontentform.h \
    $$PWD/frameliststyleditemdelegate.h \
    $$PWD/framelistviewproxystyle.h \
    $$PWD/importfiledialog.h \
    $$PWD/iraspatreeview.h \
    $$PWD/mainwindow.h \
    $$PWD/moviemaker.h \
    $$PWD/openiraspaformatdialog.h \
    $$PWD/projecttreeview.h \
    $$PWD/projecttreeviewproxystyle.h \
    $$PWD/projecttreeviewstyleditemdelegate.h \
    $$PWD/qdoubledial.h \
    $$PWD/renderstackedwidget.h \
    $$PWD/aboutdialog.h \
    $$PWD/iraspatoolbar.h \
    $$PWD/mastertoolbar.h \
    $$PWD/cameratreewidgetcontroller.h \
    $$PWD/savecifformatdialog.h \
    $$PWD/saveiraspaformatdialog.h \
    $$PWD/savemmcifformatdialog.h \
    $$PWD/savemoviedialog.h \
    $$PWD/savepdbformatdialog.h \
    $$PWD/savepicturedialog.h \
    $$PWD/saveposcarformatdialog.h \
    $$PWD/savexyzformatdialog.h \
    $$PWD/scenetreeview.h \
    $$PWD/framelistview.h \
    $$PWD/detailtabviewcontroller.h \
    $$PWD/cameracameraform.h \
    $$PWD/iraspamainwindowconsumerprotocol.h \
    $$PWD/atomtreeview.h \
    $$PWD/atomtreeviewmodel.h \
    $$PWD/framelistviewmodel.h \
    $$PWD/scenetreeviewmodel.h \
    $$PWD/informationpanelview.h \
    $$PWD/scenetreeviewproxystyle.h \
    $$PWD/scenetreeviewstyleditemdelegate.h \
    $$PWD/textfield.h \
    $$PWD/masterstackedwidget.h \
    $$PWD/cellcellform.h \
    $$PWD/celltreewidgetcontroller.h \
    $$PWD/customintspinbox.h \
    $$PWD/customdoublespinbox.h \
    $$PWD/appearanceatomsform.h \
    $$PWD/appearancetreewidgetcontroller.h \
    $$PWD/appearancebondsform.h \
    $$PWD/appearanceunitcellform.h \
    $$PWD/appearanceadsorptionsurfaceform.h \
    $$PWD/appearanceannotationform.h \
    $$PWD/cameraselectionform.h \
    $$PWD/cameralightsform.h \
    $$PWD/camerapicturesform.h \
    $$PWD/camerabackgroundform.h \
    $$PWD/elementsform.h \
    $$PWD/infocreatorform.h \
    $$PWD/infocreationform.h \
    $$PWD/infochemicalform.h \
    $$PWD/infocitationform.h \
    $$PWD/cellstructuralform.h \
    $$PWD/cellsymmetryform.h \
    $$PWD/infotreewidgetcontroller.h \
    $$PWD/qdoubleslider.h \
    $$PWD/selectcolorbutton.h \
    $$PWD/qcolorhash.h \
    $$PWD/fixedaspectratiolayoutitem.h \
    $$PWD/bondlistviewmodel.h \
    $$PWD/bondlistview.h \
    $$PWD/bondlistviewsliderstyleditemdelegate.h \
    $$PWD/qtsegmentcontrol.h \
    $$PWD/elementlistwidgetcontroller.h \
    $$PWD/linenumberplaintextedit.h \
    $$PWD/logviewcontroller.h \
    $$PWD/pythonviewcontroller.h \
    $$PWD/helpbrowser.h \
    $$PWD/helpwidget.h \
    $$PWD/projecttreeviewmodel.h

FORMS += \
    $$PWD/appearanceprimitiveform.ui \
    $$PWD/celltransformcontentform.ui \
    $$PWD/mainwindow.ui \
    $$PWD/aboutdialog.ui \
    $$PWD/cameracameraform.ui \
    $$PWD/cellcellform.ui \
    $$PWD/appearanceatomsform.ui \
    $$PWD/appearancebondsform.ui \
    $$PWD/appearanceunitcellform.ui \
    $$PWD/appearanceadsorptionsurfaceform.ui \
    $$PWD/appearanceannotationform.ui \
    $$PWD/cameraselectionform.ui \
    $$PWD/cameralightsform.ui \
    $$PWD/camerapicturesform.ui \
    $$PWD/camerabackgroundform.ui \
    $$PWD/elementsform.ui \
    $$PWD/infocreatorform.ui \
    $$PWD/infocreationform.ui \
    $$PWD/infochemicalform.ui \
    $$PWD/infocitationform.ui \
    $$PWD/cellstructuralform.ui \
    $$PWD/cellsymmetryform.ui
