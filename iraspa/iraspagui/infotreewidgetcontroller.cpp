/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
    D.Dubbeldam@uva.nl            https://www.uva.nl/en/profile/d/u/d.dubbeldam/d.dubbeldam.html
    S.Calero@tue.nl               https://www.tue.nl/en/research/researchers/sofia-calero/
    t.j.h.vlugt@tudelft.nl        http://homepage.tudelft.nl/v9k6y

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************************************************************/

#include "infotreewidgetcontroller.h"
#include <cmath>
#include <iostream>
#include <set>
#include <QLabel>
#include <QComboBox>
#include <QHash>
#include <mathkit.h>
#include <renderkit.h>
#include <foundationkit.h>
#include "textfield.h"

InfoTreeWidgetController::InfoTreeWidgetController(QWidget* parent): QTreeWidget(parent),
    _infoCreatorForm(new InfoCreatorForm),
    _infoCreationForm(new InfoCreationForm),
    _infoChemicalForm(new InfoChemicalForm),
    _infoCitationForm(new InfoCitationForm)
{
  //this->viewport()->setMouseTracking(true);
  this->setHeaderHidden(true);
  this->setRootIsDecorated(true);
  this->setFrameStyle(QFrame::NoFrame);
  this->setAnimated(true);
  this->setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
  this->setExpandsOnDoubleClick(false);
  this->setIndentation(0);
  this->setSelectionMode(QAbstractItemView::NoSelection);



  // Creator
  //==========================================================================================================
  QTreeWidgetItem* creatorItem = new QTreeWidgetItem(this);
  this->addTopLevelItem(creatorItem);

  pushButtonCreator = new QPushButton(tr("Creator"),this);
  pushButtonCreator->setIcon(QIcon(":/iraspa/collapsed.png"));
  pushButtonCreator->setStyleSheet("text-align:left;");
  setItemWidget(creatorItem,0,pushButtonCreator);

  QTreeWidgetItem *childCreatorItem = new QTreeWidgetItem(creatorItem);
  this->setItemWidget(childCreatorItem,0, _infoCreatorForm);

  pushButtonCreator->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  QFontMetrics fm(font());
  pushButtonCreator->resize(size().width(), fm.height());

  _infoCreatorForm->countryComboBox->insertItem(0,"Afghanistan");
  _infoCreatorForm->countryComboBox->insertItem(1,"Albania");
  _infoCreatorForm->countryComboBox->insertItem(2,"Algeria");
  _infoCreatorForm->countryComboBox->insertItem(3,"American Samoa");
  _infoCreatorForm->countryComboBox->insertItem(4,"Andorra");
  _infoCreatorForm->countryComboBox->insertItem(5,"Angola");
  _infoCreatorForm->countryComboBox->insertItem(6,"Anguilla");
  _infoCreatorForm->countryComboBox->insertItem(7,"Antarctica");
  _infoCreatorForm->countryComboBox->insertItem(8,"Antigua & Barbuda");
  _infoCreatorForm->countryComboBox->insertItem(9,"Argentina");
  _infoCreatorForm->countryComboBox->insertItem(10,"Armenia");
  _infoCreatorForm->countryComboBox->insertItem(11,"Aruba");
  _infoCreatorForm->countryComboBox->insertItem(12,"Ascension Island");
  _infoCreatorForm->countryComboBox->insertItem(13,"Australia");
  _infoCreatorForm->countryComboBox->insertItem(14,"Austria");
  _infoCreatorForm->countryComboBox->insertItem(15,"Azerbaijan");
  _infoCreatorForm->countryComboBox->insertItem(16,"Bahamas");
  _infoCreatorForm->countryComboBox->insertItem(17,"Bahrain");
  _infoCreatorForm->countryComboBox->insertItem(18,"Bangladesh");
  _infoCreatorForm->countryComboBox->insertItem(19,"Barbados");
  _infoCreatorForm->countryComboBox->insertItem(20,"Belarus");
  _infoCreatorForm->countryComboBox->insertItem(21,"Belgium");
  _infoCreatorForm->countryComboBox->insertItem(22,"Belize");
  _infoCreatorForm->countryComboBox->insertItem(23,"Benin");
  _infoCreatorForm->countryComboBox->insertItem(24,"Bermuda");
  _infoCreatorForm->countryComboBox->insertItem(25,"Bhutan");
  _infoCreatorForm->countryComboBox->insertItem(26,"Bolivia");
  _infoCreatorForm->countryComboBox->insertItem(27,"Bosnia & Herzegovina");
  _infoCreatorForm->countryComboBox->insertItem(28,"Botswana");
  _infoCreatorForm->countryComboBox->insertItem(29,"Bouvet Island");
  _infoCreatorForm->countryComboBox->insertItem(30,"Brazil");
  _infoCreatorForm->countryComboBox->insertItem(31,"British Indian Ocean Territory");
  _infoCreatorForm->countryComboBox->insertItem(32,"British Virgin Islands");
  _infoCreatorForm->countryComboBox->insertItem(33,"Brunei");
  _infoCreatorForm->countryComboBox->insertItem(34,"Bulgaria");
  _infoCreatorForm->countryComboBox->insertItem(35,"Burkina Faso");
  _infoCreatorForm->countryComboBox->insertItem(36,"Burundi");
  _infoCreatorForm->countryComboBox->insertItem(37,"Cambodia");
  _infoCreatorForm->countryComboBox->insertItem(38,"Cameroon");
  _infoCreatorForm->countryComboBox->insertItem(39,"Canada");
  _infoCreatorForm->countryComboBox->insertItem(40,"Canary Islands");
  _infoCreatorForm->countryComboBox->insertItem(41,"Cape Verde");
  _infoCreatorForm->countryComboBox->insertItem(42,"Caribbean Netherlands");
  _infoCreatorForm->countryComboBox->insertItem(43,"Cayman Islands");
  _infoCreatorForm->countryComboBox->insertItem(44,"Central African Republic");
  _infoCreatorForm->countryComboBox->insertItem(45,"Ceuta & Melilla");
  _infoCreatorForm->countryComboBox->insertItem(46,"Chad");
  _infoCreatorForm->countryComboBox->insertItem(47,"Chile");
  _infoCreatorForm->countryComboBox->insertItem(48,"China mainland");
  _infoCreatorForm->countryComboBox->insertItem(49,"Christmas Island");
  _infoCreatorForm->countryComboBox->insertItem(50,"Clipperton Island");
  _infoCreatorForm->countryComboBox->insertItem(51,"Cocos (Keeling) Islands");
  _infoCreatorForm->countryComboBox->insertItem(52,"Colombia");
  _infoCreatorForm->countryComboBox->insertItem(53,"Comoros");
  _infoCreatorForm->countryComboBox->insertItem(54,"Congo - Brazzaville");
  _infoCreatorForm->countryComboBox->insertItem(55,"Congo - Kinshasa");
  _infoCreatorForm->countryComboBox->insertItem(56,"Cook Islands");
  _infoCreatorForm->countryComboBox->insertItem(57,"Costa Rica");
  _infoCreatorForm->countryComboBox->insertItem(58,"Croatia");
  _infoCreatorForm->countryComboBox->insertItem(59,"Cuba");
  _infoCreatorForm->countryComboBox->insertItem(60,"Curaçao");
  _infoCreatorForm->countryComboBox->insertItem(61,"Cyprus");
  _infoCreatorForm->countryComboBox->insertItem(62,"Czechia");
  _infoCreatorForm->countryComboBox->insertItem(63,"Côte d’Ivoire");
  _infoCreatorForm->countryComboBox->insertItem(64,"Denmark");
  _infoCreatorForm->countryComboBox->insertItem(65,"Diego Garcia");
  _infoCreatorForm->countryComboBox->insertItem(66,"Djibouti");
  _infoCreatorForm->countryComboBox->insertItem(67,"Dominica");
  _infoCreatorForm->countryComboBox->insertItem(68,"Dominican Republic");
  _infoCreatorForm->countryComboBox->insertItem(69,"Ecuador");
  _infoCreatorForm->countryComboBox->insertItem(70,"Egypt");
  _infoCreatorForm->countryComboBox->insertItem(71,"El Salvador");
  _infoCreatorForm->countryComboBox->insertItem(72,"Equatorial Guinea");
  _infoCreatorForm->countryComboBox->insertItem(73,"Eritrea");
  _infoCreatorForm->countryComboBox->insertItem(74,"Estonia");
  _infoCreatorForm->countryComboBox->insertItem(75,"Eswatini");
  _infoCreatorForm->countryComboBox->insertItem(76,"Ethiopia");
  _infoCreatorForm->countryComboBox->insertItem(77,"Falkland Islands");
  _infoCreatorForm->countryComboBox->insertItem(78,"Faroe Islands");
  _infoCreatorForm->countryComboBox->insertItem(79,"Fiji");
  _infoCreatorForm->countryComboBox->insertItem(80,"Finland");
  _infoCreatorForm->countryComboBox->insertItem(81,"France");
  _infoCreatorForm->countryComboBox->insertItem(82,"French Guiana");
  _infoCreatorForm->countryComboBox->insertItem(83,"French Polynesia");
  _infoCreatorForm->countryComboBox->insertItem(84,"French Southern Territories");
  _infoCreatorForm->countryComboBox->insertItem(85,"Gabon");
  _infoCreatorForm->countryComboBox->insertItem(86,"Gambia");
  _infoCreatorForm->countryComboBox->insertItem(87,"Georgia");
  _infoCreatorForm->countryComboBox->insertItem(88,"Germany");
  _infoCreatorForm->countryComboBox->insertItem(89,"Ghana");
  _infoCreatorForm->countryComboBox->insertItem(90,"Gibraltar");
  _infoCreatorForm->countryComboBox->insertItem(91,"Greece");
  _infoCreatorForm->countryComboBox->insertItem(92,"Greenland");
  _infoCreatorForm->countryComboBox->insertItem(93,"Grenada");
  _infoCreatorForm->countryComboBox->insertItem(94,"Guadeloupe");
  _infoCreatorForm->countryComboBox->insertItem(95,"Guam");
  _infoCreatorForm->countryComboBox->insertItem(96,"Guatemala");
  _infoCreatorForm->countryComboBox->insertItem(97,"Guernsey");
  _infoCreatorForm->countryComboBox->insertItem(98,"Guinea");
  _infoCreatorForm->countryComboBox->insertItem(99,"Guinea-Bissau");
  _infoCreatorForm->countryComboBox->insertItem(100,"Guyana");
  _infoCreatorForm->countryComboBox->insertItem(101,"Haiti");
  _infoCreatorForm->countryComboBox->insertItem(102,"Heard & McDonald Islands");
  _infoCreatorForm->countryComboBox->insertItem(103,"Honduras");
  _infoCreatorForm->countryComboBox->insertItem(104,"Hong Kong");
  _infoCreatorForm->countryComboBox->insertItem(105,"Hungary");
  _infoCreatorForm->countryComboBox->insertItem(106,"Iceland");
  _infoCreatorForm->countryComboBox->insertItem(107,"India");
  _infoCreatorForm->countryComboBox->insertItem(108,"Indonesia");
  _infoCreatorForm->countryComboBox->insertItem(109,"Iran");
  _infoCreatorForm->countryComboBox->insertItem(110,"Iraq");
  _infoCreatorForm->countryComboBox->insertItem(111,"Ireland");
  _infoCreatorForm->countryComboBox->insertItem(112,"Isle of Man");
  _infoCreatorForm->countryComboBox->insertItem(113,"Israel");
  _infoCreatorForm->countryComboBox->insertItem(114,"Italy");
  _infoCreatorForm->countryComboBox->insertItem(115,"Jamaica");
  _infoCreatorForm->countryComboBox->insertItem(116,"Japan");
  _infoCreatorForm->countryComboBox->insertItem(117,"Jersey");
  _infoCreatorForm->countryComboBox->insertItem(118,"Jordan");
  _infoCreatorForm->countryComboBox->insertItem(119,"Kazakhstan");
  _infoCreatorForm->countryComboBox->insertItem(120,"Kenya");
  _infoCreatorForm->countryComboBox->insertItem(121,"Kiribati");
  _infoCreatorForm->countryComboBox->insertItem(122,"Kosovo");
  _infoCreatorForm->countryComboBox->insertItem(123,"Kuwait");
  _infoCreatorForm->countryComboBox->insertItem(124,"Kyrgyzstan");
  _infoCreatorForm->countryComboBox->insertItem(125,"Laos");
  _infoCreatorForm->countryComboBox->insertItem(126,"Latvia");
  _infoCreatorForm->countryComboBox->insertItem(127,"Lebanon");
  _infoCreatorForm->countryComboBox->insertItem(128,"Lesotho");
  _infoCreatorForm->countryComboBox->insertItem(129,"Liberia");
  _infoCreatorForm->countryComboBox->insertItem(130,"Libya");
  _infoCreatorForm->countryComboBox->insertItem(131,"Liechtenstein");
  _infoCreatorForm->countryComboBox->insertItem(132,"Lithuania");
  _infoCreatorForm->countryComboBox->insertItem(133,"Luxembourg");
  _infoCreatorForm->countryComboBox->insertItem(134,"Macao");
  _infoCreatorForm->countryComboBox->insertItem(135,"Madagascar");
  _infoCreatorForm->countryComboBox->insertItem(136,"Malawi");
  _infoCreatorForm->countryComboBox->insertItem(137,"Malaysia");
  _infoCreatorForm->countryComboBox->insertItem(138,"Maldives");
  _infoCreatorForm->countryComboBox->insertItem(139,"Mali");
  _infoCreatorForm->countryComboBox->insertItem(140,"Malta");
  _infoCreatorForm->countryComboBox->insertItem(141,"Marshall Islands");
  _infoCreatorForm->countryComboBox->insertItem(142,"Martinique");
  _infoCreatorForm->countryComboBox->insertItem(143,"Mauritania");
  _infoCreatorForm->countryComboBox->insertItem(144,"Mauritius");
  _infoCreatorForm->countryComboBox->insertItem(145,"Mayotte");
  _infoCreatorForm->countryComboBox->insertItem(146,"Mexico");
  _infoCreatorForm->countryComboBox->insertItem(147,"Micronesia");
  _infoCreatorForm->countryComboBox->insertItem(148,"Moldova");
  _infoCreatorForm->countryComboBox->insertItem(149,"Monaco");
  _infoCreatorForm->countryComboBox->insertItem(150,"Mongolia");
  _infoCreatorForm->countryComboBox->insertItem(151,"Montenegro");
  _infoCreatorForm->countryComboBox->insertItem(152,"Montserrat");
  _infoCreatorForm->countryComboBox->insertItem(153,"Morocco");
  _infoCreatorForm->countryComboBox->insertItem(154,"Mozambique");
  _infoCreatorForm->countryComboBox->insertItem(155,"Myanmar (Burma)");
  _infoCreatorForm->countryComboBox->insertItem(156,"Namibia");
  _infoCreatorForm->countryComboBox->insertItem(157,"Nauru");
  _infoCreatorForm->countryComboBox->insertItem(158,"Nepal");
  _infoCreatorForm->countryComboBox->insertItem(159,"Netherlands");
  _infoCreatorForm->countryComboBox->insertItem(160,"New Caledonia");
  _infoCreatorForm->countryComboBox->insertItem(161,"New Zealand");
  _infoCreatorForm->countryComboBox->insertItem(162,"Nicaragua");
  _infoCreatorForm->countryComboBox->insertItem(163,"Niger");
  _infoCreatorForm->countryComboBox->insertItem(164,"Nigeria");
  _infoCreatorForm->countryComboBox->insertItem(165,"Niue");
  _infoCreatorForm->countryComboBox->insertItem(166,"Norfolk Island");
  _infoCreatorForm->countryComboBox->insertItem(167,"North Korea");
  _infoCreatorForm->countryComboBox->insertItem(168,"North Macedonia");
  _infoCreatorForm->countryComboBox->insertItem(169,"Northern Mariana Islands");
  _infoCreatorForm->countryComboBox->insertItem(170,"Norway");
  _infoCreatorForm->countryComboBox->insertItem(171,"Oman");
  _infoCreatorForm->countryComboBox->insertItem(172,"Pakistan");
  _infoCreatorForm->countryComboBox->insertItem(173,"Palau");
  _infoCreatorForm->countryComboBox->insertItem(174,"Palestinian Territories");
  _infoCreatorForm->countryComboBox->insertItem(175,"Panama");
  _infoCreatorForm->countryComboBox->insertItem(176,"Papua New Guinea");
  _infoCreatorForm->countryComboBox->insertItem(177,"Paraguay");
  _infoCreatorForm->countryComboBox->insertItem(178,"Peru");
  _infoCreatorForm->countryComboBox->insertItem(179,"Philippines");
  _infoCreatorForm->countryComboBox->insertItem(180,"Pitcairn Islands");
  _infoCreatorForm->countryComboBox->insertItem(181,"Poland");
  _infoCreatorForm->countryComboBox->insertItem(182,"Portugal");
  _infoCreatorForm->countryComboBox->insertItem(183,"Puerto Rico");
  _infoCreatorForm->countryComboBox->insertItem(184,"Qatar");
  _infoCreatorForm->countryComboBox->insertItem(185,"Romania");
  _infoCreatorForm->countryComboBox->insertItem(186,"Russia");
  _infoCreatorForm->countryComboBox->insertItem(187,"Rwanda");
  _infoCreatorForm->countryComboBox->insertItem(188,"Réunion");
  _infoCreatorForm->countryComboBox->insertItem(189,"Samoa");
  _infoCreatorForm->countryComboBox->insertItem(190,"San Marino");
  _infoCreatorForm->countryComboBox->insertItem(191,"Saudi Arabia");
  _infoCreatorForm->countryComboBox->insertItem(192,"Senegal");
  _infoCreatorForm->countryComboBox->insertItem(193,"Serbia");
  _infoCreatorForm->countryComboBox->insertItem(194,"Seychelles");
  _infoCreatorForm->countryComboBox->insertItem(195,"Sierra Leone");
  _infoCreatorForm->countryComboBox->insertItem(196,"Singapore");
  _infoCreatorForm->countryComboBox->insertItem(197,"Sint Maarten");
  _infoCreatorForm->countryComboBox->insertItem(198,"Slovakia");
  _infoCreatorForm->countryComboBox->insertItem(199,"Slovenia");
  _infoCreatorForm->countryComboBox->insertItem(200,"So. Georgia & So. Sandwich Isl.");
  _infoCreatorForm->countryComboBox->insertItem(201,"Solomon Islands");
  _infoCreatorForm->countryComboBox->insertItem(202,"Somalia");
  _infoCreatorForm->countryComboBox->insertItem(203,"South Africa");
  _infoCreatorForm->countryComboBox->insertItem(204,"South Korea");
  _infoCreatorForm->countryComboBox->insertItem(205,"South Sudan");
  _infoCreatorForm->countryComboBox->insertItem(206,"Spain");
  _infoCreatorForm->countryComboBox->insertItem(207,"Sri Lanka");
  _infoCreatorForm->countryComboBox->insertItem(208,"St. Barthélemy");
  _infoCreatorForm->countryComboBox->insertItem(209,"St. Helena");
  _infoCreatorForm->countryComboBox->insertItem(210,"St. Kitts & Nevis");
  _infoCreatorForm->countryComboBox->insertItem(211,"St. Lucia");
  _infoCreatorForm->countryComboBox->insertItem(212,"St. Martin");
  _infoCreatorForm->countryComboBox->insertItem(213,"St. Pierre & Miquelon");
  _infoCreatorForm->countryComboBox->insertItem(214,"St. Vincent & Grenadines");
  _infoCreatorForm->countryComboBox->insertItem(215,"Sudan");
  _infoCreatorForm->countryComboBox->insertItem(216,"Suriname");
  _infoCreatorForm->countryComboBox->insertItem(217,"Svalbard & Jan Mayen");
  _infoCreatorForm->countryComboBox->insertItem(218,"Sweden");
  _infoCreatorForm->countryComboBox->insertItem(219,"Switzerland");
  _infoCreatorForm->countryComboBox->insertItem(220,"Syria");
  _infoCreatorForm->countryComboBox->insertItem(221,"São Tomé & Príncipe");
  _infoCreatorForm->countryComboBox->insertItem(222,"Taiwan");
  _infoCreatorForm->countryComboBox->insertItem(223,"Tajikistan");
  _infoCreatorForm->countryComboBox->insertItem(224,"Tanzania");
  _infoCreatorForm->countryComboBox->insertItem(225,"Thailand");
  _infoCreatorForm->countryComboBox->insertItem(226,"Timor-Leste");
  _infoCreatorForm->countryComboBox->insertItem(227,"Togo");
  _infoCreatorForm->countryComboBox->insertItem(228,"Tokelau");
  _infoCreatorForm->countryComboBox->insertItem(229,"Tonga");
  _infoCreatorForm->countryComboBox->insertItem(230,"Trinidad & Tobago");
  _infoCreatorForm->countryComboBox->insertItem(231,"Tristan da Cunha");
  _infoCreatorForm->countryComboBox->insertItem(232,"Tunisia");
  _infoCreatorForm->countryComboBox->insertItem(233,"Turkey");
  _infoCreatorForm->countryComboBox->insertItem(234,"Turkmenistan");
  _infoCreatorForm->countryComboBox->insertItem(235,"Turks & Caicos Islands");
  _infoCreatorForm->countryComboBox->insertItem(236,"Tuvalu");
  _infoCreatorForm->countryComboBox->insertItem(237,"U.S. Outlying Islands");
  _infoCreatorForm->countryComboBox->insertItem(238,"U.S. Virgin Islands");
  _infoCreatorForm->countryComboBox->insertItem(239,"Uganda");
  _infoCreatorForm->countryComboBox->insertItem(240,"Ukraine");
  _infoCreatorForm->countryComboBox->insertItem(241,"United Arab Emirates");
  _infoCreatorForm->countryComboBox->insertItem(242,"United Kingdom");
  _infoCreatorForm->countryComboBox->insertItem(243,"United States");
  _infoCreatorForm->countryComboBox->insertItem(244,"Uruguay");
  _infoCreatorForm->countryComboBox->insertItem(245,"Uzbekistan");
  _infoCreatorForm->countryComboBox->insertItem(246,"Vanuatu");
  _infoCreatorForm->countryComboBox->insertItem(247,"Vatican City");
  _infoCreatorForm->countryComboBox->insertItem(248,"Venezuela");
  _infoCreatorForm->countryComboBox->insertItem(249,"Vietnam");
  _infoCreatorForm->countryComboBox->insertItem(250,"Wallis & Futuna");
  _infoCreatorForm->countryComboBox->insertItem(251,"Western Sahara");
  _infoCreatorForm->countryComboBox->insertItem(252,"Yemen");
  _infoCreatorForm->countryComboBox->insertItem(253,"Zambia");
  _infoCreatorForm->countryComboBox->insertItem(254,"Zimbabwe");
  _infoCreatorForm->countryComboBox->insertItem(255,"Åland Islands");

  QObject::connect(_infoCreatorForm->firstNameLineEdit, &CustomLineEdit::textOnEditingFinished,this,&InfoTreeWidgetController::setAuthorFirstName);
  QObject::connect(_infoCreatorForm->middleNameLineEdit, &CustomLineEdit::textOnEditingFinished,this,&InfoTreeWidgetController::setAuthorMiddleName);
  QObject::connect(_infoCreatorForm->lastNameLineEdit, &CustomLineEdit::textOnEditingFinished,this,&InfoTreeWidgetController::setAuthorLastName);
  QObject::connect(_infoCreatorForm->orchidLineEdit, &CustomLineEdit::textOnEditingFinished,this,&InfoTreeWidgetController::setAuthorOrchidID);
  QObject::connect(_infoCreatorForm->researcherIDLineEdit, &CustomLineEdit::textOnEditingFinished,this,&InfoTreeWidgetController::setAuthorResearcherID);
  QObject::connect(_infoCreatorForm->universityNameLineEdit, &CustomLineEdit::textOnEditingFinished,this,&InfoTreeWidgetController::setAuthorAffiliationUniversityName);
  QObject::connect(_infoCreatorForm->facultyNameLineEdit, &CustomLineEdit::textOnEditingFinished,this,&InfoTreeWidgetController::setAuthorAffiliationFacultyName);
  QObject::connect(_infoCreatorForm->instituteNameLineEdit, &CustomLineEdit::textOnEditingFinished,this,&InfoTreeWidgetController::setAuthorAffiliationInstituteName);
  QObject::connect(_infoCreatorForm->cityNameLineEdit, &CustomLineEdit::textOnEditingFinished,this,&InfoTreeWidgetController::setAuthorAffiliationCityName);
  QObject::connect(_infoCreatorForm->countryComboBox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged),this,&InfoTreeWidgetController::setAuthorAffiliationCountryName);



  // Creation
  //==========================================================================================================
  QTreeWidgetItem* creationItem = new QTreeWidgetItem(this);
  this->addTopLevelItem(creationItem);

  pushButtonCreation = new QPushButton(tr("Creation"),this);
  pushButtonCreation->setIcon(QIcon(":/iraspa/collapsed.png"));
  pushButtonCreation->setStyleSheet("text-align:left;");
  setItemWidget(creationItem,0,pushButtonCreation);

  QTreeWidgetItem *childCreationItem = new QTreeWidgetItem(creationItem);
  this->setItemWidget(childCreationItem,0, _infoCreationForm);

  QObject::connect(_infoCreationForm->methodComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this](int index) {
      switch(index)
      {
      case 0:
      case 1:
        _infoCreationForm->stackedWidget->setCurrentIndex(0);
        break;
      case 2:
        _infoCreationForm->stackedWidget->setCurrentIndex(1);
        break;
      }
  });

  pushButtonCreation->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  pushButtonCreation->resize(size().width(), fm.height());

  _infoCreationForm->temperatureComboBox->insertItem(0,tr("Kelvin"));
  _infoCreationForm->temperatureComboBox->insertItem(1,tr("Celsius"));

  _infoCreationForm->pressureComboBox->insertItem(0,tr("Pascal"));
  _infoCreationForm->pressureComboBox->insertItem(1,tr("Bar"));

  _infoCreationForm->methodComboBox->insertItem(0,tr("Unknown"));
  _infoCreationForm->methodComboBox->insertItem(1,tr("Simulation"));
  _infoCreationForm->methodComboBox->insertItem(2,tr("Experiment"));


  _infoCreationForm->relaxUnitCellComboBox->insertItem(0,tr("Unknown"));
  _infoCreationForm->relaxUnitCellComboBox->insertItem(1,tr("Cell and Edge-Lengths Free"));
  _infoCreationForm->relaxUnitCellComboBox->insertItem(2,tr("Fixed Angles; Isotropic Edge-Lengths Free"));
  _infoCreationForm->relaxUnitCellComboBox->insertItem(3,tr("Fixed Angles; Anisotropic Edge-Lengths Free"));
  _infoCreationForm->relaxUnitCellComboBox->insertItem(4,tr("Fixed α-, β- and γ-Angles and Edge-Lengths Free"));
  _infoCreationForm->relaxUnitCellComboBox->insertItem(5,tr("Fixed Volume; Shape Free"));
  _infoCreationForm->relaxUnitCellComboBox->insertItem(6,tr("Fixed Cell"));

  _infoCreationForm->positionsSoftwarePackageComboBox->insertItem(0,tr("Unknown"));
  _infoCreationForm->positionsSoftwarePackageComboBox->insertItem(1,tr("Experimental"));
  _infoCreationForm->positionsSoftwarePackageComboBox->insertItem(2,tr("Gaussian"));
  _infoCreationForm->positionsSoftwarePackageComboBox->insertItem(3,tr("Turbomol"));
  _infoCreationForm->positionsSoftwarePackageComboBox->insertItem(4,tr("VASP"));
  _infoCreationForm->positionsSoftwarePackageComboBox->insertItem(5,tr("Spartan"));
  _infoCreationForm->positionsSoftwarePackageComboBox->insertItem(6,tr("CP2K"));
  _infoCreationForm->positionsSoftwarePackageComboBox->insertItem(7,tr("Classical"));

  _infoCreationForm->positionsAlgorithmComboBox->insertItem(0,tr("Unknown"));
  _infoCreationForm->positionsAlgorithmComboBox->insertItem(1,tr("None"));
  _infoCreationForm->positionsAlgorithmComboBox->insertItem(2,tr("Simplex"));
  _infoCreationForm->positionsAlgorithmComboBox->insertItem(3,tr("Simulated Annealing"));
  _infoCreationForm->positionsAlgorithmComboBox->insertItem(4,tr("Genetic Algorithm"));
  _infoCreationForm->positionsAlgorithmComboBox->insertItem(5,tr("Steepest Descent"));
  _infoCreationForm->positionsAlgorithmComboBox->insertItem(6,tr("Conjugate Gradient"));
  _infoCreationForm->positionsAlgorithmComboBox->insertItem(7,tr("Quasi-Newton"));
  _infoCreationForm->positionsAlgorithmComboBox->insertItem(8,tr("Newton-Rhapson"));
  _infoCreationForm->positionsAlgorithmComboBox->insertItem(9,tr("Mode Following"));


  _infoCreationForm->positionsEigenvaluesComboBox->insertItem(0,tr("Unknown"));
  _infoCreationForm->positionsEigenvaluesComboBox->insertItem(1,tr("None"));
  _infoCreationForm->positionsEigenvaluesComboBox->insertItem(2,tr("All Positive"));
  _infoCreationForm->positionsEigenvaluesComboBox->insertItem(3,tr("First Order Sadle-Point"));
  _infoCreationForm->positionsEigenvaluesComboBox->insertItem(4,tr("Second Order Sadle-Point"));
  _infoCreationForm->positionsEigenvaluesComboBox->insertItem(5,tr("Some Small Negative Eigenvalues"));
  _infoCreationForm->positionsEigenvaluesComboBox->insertItem(6,tr("Some Significant Negative Eigenvalues"));
  _infoCreationForm->positionsEigenvaluesComboBox->insertItem(7,tr("Many Negative Eigenvalues"));


  _infoCreationForm->positionsForceFieldComboBox->insertItem(0,tr("Unknown"));
  _infoCreationForm->positionsForceFieldComboBox->insertItem(1,tr("Ab Initio"));
  _infoCreationForm->positionsForceFieldComboBox->insertItem(2,tr("UFF"));
  _infoCreationForm->positionsForceFieldComboBox->insertItem(3,tr("DREIDING"));
  _infoCreationForm->positionsForceFieldComboBox->insertItem(4,tr("MM3"));
  _infoCreationForm->positionsForceFieldComboBox->insertItem(5,tr("MM4"));

  _infoCreationForm->positionsForceFieldDetailsComboBox->insertItem(0,tr("Unknown"));

  _infoCreationForm->chargeSoftwarePackageComboBox->insertItem(0,tr("Unknown"));
  _infoCreationForm->chargeSoftwarePackageComboBox->insertItem(1,tr("Experimental"));
  _infoCreationForm->chargeSoftwarePackageComboBox->insertItem(2,tr("Gaussian"));
  _infoCreationForm->chargeSoftwarePackageComboBox->insertItem(3,tr("Turbomol"));
  _infoCreationForm->chargeSoftwarePackageComboBox->insertItem(4,tr("VASP"));
  _infoCreationForm->chargeSoftwarePackageComboBox->insertItem(5,tr("Spartan"));
  _infoCreationForm->chargeSoftwarePackageComboBox->insertItem(6,tr("CP2K"));
  _infoCreationForm->chargeSoftwarePackageComboBox->insertItem(7,tr("Classical"));

  _infoCreationForm->chargeAlgorithmComboBox->insertItem(0,tr("Unknown"));
  _infoCreationForm->chargeAlgorithmComboBox->insertItem(1,tr("REPEAT"));
  _infoCreationForm->chargeAlgorithmComboBox->insertItem(2,tr("CHELPG"));
  _infoCreationForm->chargeAlgorithmComboBox->insertItem(3,tr("CHELP"));
  _infoCreationForm->chargeAlgorithmComboBox->insertItem(4,tr("RESP"));
  _infoCreationForm->chargeAlgorithmComboBox->insertItem(5,tr("Mulliken"));
  _infoCreationForm->chargeAlgorithmComboBox->insertItem(6,tr("Merz-Kollman"));
  _infoCreationForm->chargeAlgorithmComboBox->insertItem(7,tr("Hirshfeld"));
  _infoCreationForm->chargeAlgorithmComboBox->insertItem(8,tr("Mulliken"));
  _infoCreationForm->chargeAlgorithmComboBox->insertItem(9,tr("Natural Bond Orbital"));
  _infoCreationForm->chargeAlgorithmComboBox->insertItem(10,tr("Qeq - Rappe and Goddard 1991"));
  _infoCreationForm->chargeAlgorithmComboBox->insertItem(11,tr("Qeq - Rick, Stuart and Berne 1994"));
  _infoCreationForm->chargeAlgorithmComboBox->insertItem(12,tr("Qeq - York and Yang 1996"));
  _infoCreationForm->chargeAlgorithmComboBox->insertItem(13,tr("Qeq - Itskoqitz and Berkowitz 1997"));
  _infoCreationForm->chargeAlgorithmComboBox->insertItem(14,tr("Qeq - Wilmer, Kim, Snurr 2012"));

  _infoCreationForm->chargeForceFieldComboBox->insertItem(0,tr("Unknown"));
  _infoCreationForm->chargeForceFieldComboBox->insertItem(1,tr("Ab Initio"));
  _infoCreationForm->chargeForceFieldComboBox->insertItem(2,tr("UFF"));
  _infoCreationForm->chargeForceFieldComboBox->insertItem(3,tr("DREIDING"));
  _infoCreationForm->chargeForceFieldComboBox->insertItem(4,tr("MM3"));
  _infoCreationForm->chargeForceFieldComboBox->insertItem(5,tr("MM4"));

  _infoCreationForm->chargeForceFieldDetailsComboBox->insertItem(0,tr("Unknown"));

  QObject::connect(_infoCreationForm->creationDateEdit, &QDateEdit::dateChanged,this,&InfoTreeWidgetController::setCreationDate);
  QObject::connect(_infoCreationForm->temperatureLineEdit, &CustomLineEdit::textOnEditingFinished,this,&InfoTreeWidgetController::setCreationTemperature);
  QObject::connect(_infoCreationForm->temperatureComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&InfoTreeWidgetController::setCreationTemperatureScale);
  QObject::connect(_infoCreationForm->pressureLineEdit, &CustomLineEdit::textOnEditingFinished,this,&InfoTreeWidgetController::setCreationPressure);
  QObject::connect(_infoCreationForm->pressureComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&InfoTreeWidgetController::setCreationPressureScale);
  QObject::connect(_infoCreationForm->methodComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&InfoTreeWidgetController::setCreationMethod);


  QObject::connect(_infoCreationForm->relaxUnitCellComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&InfoTreeWidgetController::setCreationRelaxUnitCell);
  QObject::connect(_infoCreationForm->positionsSoftwarePackageComboBox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged),this,&InfoTreeWidgetController::setCreationAtomicPositionsSoftwarePackage);
  QObject::connect(_infoCreationForm->positionsAlgorithmComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&InfoTreeWidgetController::setCreationAtomicPositionsAlgorithm);
  QObject::connect(_infoCreationForm->positionsEigenvaluesComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&InfoTreeWidgetController::setCreationAtomicPositionsEigenvalues);
  QObject::connect(_infoCreationForm->positionsForceFieldComboBox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged),this,&InfoTreeWidgetController::setCreationAtomicPositionsForceField);
  QObject::connect(_infoCreationForm->positionsForceFieldDetailsComboBox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged),this,&InfoTreeWidgetController::setCreationAtomicPositionsForceFieldDetails);

  QObject::connect(_infoCreationForm->chargeSoftwarePackageComboBox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged),this,&InfoTreeWidgetController::setCreationAtomicChargesSoftwarePackage);
  QObject::connect(_infoCreationForm->chargeAlgorithmComboBox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged),this,&InfoTreeWidgetController::setCreationAtomicChargesAlgorithm);
  QObject::connect(_infoCreationForm->chargeForceFieldComboBox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged),this,&InfoTreeWidgetController::setCreationAtomicChargesForceField);
  QObject::connect(_infoCreationForm->chargeForceFieldDetailsComboBox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged),this,&InfoTreeWidgetController::setCreationAtomicChargesForceFieldDetails);


  QObject::connect(_infoCreationForm->experimentalRadiationTypeLineEdit, &CustomLineEdit::textOnEditingFinished,this,&InfoTreeWidgetController::setCreationExperimentalRadiationType);
  QObject::connect(_infoCreationForm->experimentalRadiationWavelengthDoubleSpinBox,static_cast<void (CustomDoubleSpinBox::*)(const QString&)>(&CustomDoubleSpinBox::textOnEditingFinished),this,&InfoTreeWidgetController::setCreationExperimentalWaveLength);
  QObject::connect(_infoCreationForm->experimentalMeasurementThetaMinDoubleSpinBox,static_cast<void (CustomDoubleSpinBox::*)(const QString&)>(&CustomDoubleSpinBox::textOnEditingFinished),this,&InfoTreeWidgetController::setCreationExperimentalMeasuermentThetaMin);
  QObject::connect(_infoCreationForm->experimentalMeasurementThetaMaxDoubleSpinBox,static_cast<void (CustomDoubleSpinBox::*)(const QString&)>(&CustomDoubleSpinBox::textOnEditingFinished),this,&InfoTreeWidgetController::setCreationExperimentalMeasuermentThetaMax);
  QObject::connect(_infoCreationForm->experimentalhminDoubleSpinBox,static_cast<void (CustomDoubleSpinBox::*)(const QString&)>(&CustomDoubleSpinBox::textOnEditingFinished),this,&InfoTreeWidgetController::setCreationExperimentalMeasuermentHMin);
  QObject::connect(_infoCreationForm->experimentalhmaxDoubleSpinBox,static_cast<void (CustomDoubleSpinBox::*)(const QString&)>(&CustomDoubleSpinBox::textOnEditingFinished),this,&InfoTreeWidgetController::setCreationExperimentalMeasuermentHMax);
  QObject::connect(_infoCreationForm->experimentalkminDoubleSpinBox,static_cast<void (CustomDoubleSpinBox::*)(const QString&)>(&CustomDoubleSpinBox::textOnEditingFinished),this,&InfoTreeWidgetController::setCreationExperimentalMeasuermentKMin);
  QObject::connect(_infoCreationForm->experimentalkmaxDoubleSpinBox,static_cast<void (CustomDoubleSpinBox::*)(const QString&)>(&CustomDoubleSpinBox::textOnEditingFinished),this,&InfoTreeWidgetController::setCreationExperimentalMeasuermentKMax);
  QObject::connect(_infoCreationForm->experimentallminDoubleSpinBox,static_cast<void (CustomDoubleSpinBox::*)(const QString&)>(&CustomDoubleSpinBox::textOnEditingFinished),this,&InfoTreeWidgetController::setCreationExperimentalMeasuermentLMin);
  QObject::connect(_infoCreationForm->experimentallmaxDoubleSpinBox,static_cast<void (CustomDoubleSpinBox::*)(const QString&)>(&CustomDoubleSpinBox::textOnEditingFinished),this,&InfoTreeWidgetController::setCreationExperimentalMeasuermentLMax);
  QObject::connect(_infoCreationForm->experimentalNumberOfReflectionsSpinBox,static_cast<void (CustomIntSpinBox::*)(const QString&)>(&CustomIntSpinBox::textOnEditingFinished),this,&InfoTreeWidgetController::setCreationExperimentalNumberOfReflections);
  QObject::connect(_infoCreationForm->experimentalSoftwareLineEdit, &CustomLineEdit::textOnEditingFinished,this,&InfoTreeWidgetController::setCreationExperimentalSoftware);
  QObject::connect(_infoCreationForm->experimentalDetailsTextEdit, &CustomPlainTextEdit::textOnEditingFinished,this,&InfoTreeWidgetController::setCreationExperimentalDetails);
  QObject::connect(_infoCreationForm->experimentalGoodnessOfFitDoubleSpinBox,static_cast<void (CustomDoubleSpinBox::*)(const QString&)>(&CustomDoubleSpinBox::textOnEditingFinished),this,&InfoTreeWidgetController::setCreationExperimentalGoodnessOfFit);
  QObject::connect(_infoCreationForm->experimentalFinalIndicesDoubleSpinBox,static_cast<void (CustomDoubleSpinBox::*)(const QString&)>(&CustomDoubleSpinBox::textOnEditingFinished),this,&InfoTreeWidgetController::setCreationExperimentalFinalIndices);
  QObject::connect(_infoCreationForm->experimentalRIndicesDoubleSpinBox,static_cast<void (CustomDoubleSpinBox::*)(const QString&)>(&CustomDoubleSpinBox::textOnEditingFinished),this,&InfoTreeWidgetController::setCreationExperimentalRIndicest);



  // Chemical
  //==========================================================================================================
  QTreeWidgetItem* chemicalItem = new QTreeWidgetItem(this);
  this->addTopLevelItem(chemicalItem);

  pushButtonChemical = new QPushButton(tr("Chemical Information"),this);
  pushButtonChemical->setIcon(QIcon(":/iraspa/collapsed.png"));
  pushButtonChemical->setStyleSheet("text-align:left;");
  setItemWidget(chemicalItem,0,pushButtonChemical);

  QTreeWidgetItem *childChemicalItem = new QTreeWidgetItem(chemicalItem);
  this->setItemWidget(childChemicalItem,0, _infoChemicalForm);

  pushButtonChemical->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  pushButtonChemical->resize(size().width(), fm.height());

  QObject::connect(_infoChemicalForm->moietyLineEdit,static_cast<void (CustomLineEdit::*)(const QString&)>(&CustomLineEdit::textOnEditingFinished),this,&InfoTreeWidgetController::setChemicalFormulaMoiety);
  QObject::connect(_infoChemicalForm->sytematicLineEdit,static_cast<void (CustomLineEdit::*)(const QString&)>(&CustomLineEdit::textOnEditingFinished),this,&InfoTreeWidgetController::setChemicalFormulaSum);
  QObject::connect(_infoChemicalForm->sumLineEdit,static_cast<void (CustomLineEdit::*)(const QString&)>(&CustomLineEdit::textOnEditingFinished),this,&InfoTreeWidgetController::setChemicalNameSystematic);


  // Citation
  //==========================================================================================================
  QTreeWidgetItem* citationItem = new QTreeWidgetItem(this);
  this->addTopLevelItem(citationItem);

  pushButtonCitation = new QPushButton(tr("Citation"),this);
  pushButtonCitation->setIcon(QIcon(":/iraspa/collapsed.png"));
  pushButtonCitation->setStyleSheet("text-align:left;");
  setItemWidget(citationItem,0,pushButtonCitation);

  QTreeWidgetItem *childCitationItem = new QTreeWidgetItem(citationItem);
  this->setItemWidget(childCitationItem,0, _infoCitationForm);

  pushButtonCitation->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  pushButtonCitation->resize(size().width(), fm.height());


  QObject::connect(_infoCitationForm->articleTitleTextEdit,static_cast<void (CustomPlainTextEdit::*)(const QString&)>(&CustomPlainTextEdit::textOnEditingFinished),this,&InfoTreeWidgetController::setCitationArticleTitle);
  QObject::connect(_infoCitationForm->journalTitleLineEdit,static_cast<void (CustomLineEdit::*)(const QString&)>(&CustomLineEdit::textOnEditingFinished),this,&InfoTreeWidgetController::setCitationJournalTitle);
  QObject::connect(_infoCitationForm->articleAuthorsTextEdit,static_cast<void (CustomPlainTextEdit::*)(const QString&)>(&CustomPlainTextEdit::textOnEditingFinished),this,&InfoTreeWidgetController::setCitationArticleAuthors);
  QObject::connect(_infoCitationForm->journalVolumeLineEdit,static_cast<void (CustomLineEdit::*)(const QString&)>(&CustomLineEdit::textOnEditingFinished),this,&InfoTreeWidgetController::setCitationJournalVolume);
  QObject::connect(_infoCitationForm->journalNumberLineEdit,static_cast<void (CustomLineEdit::*)(const QString&)>(&CustomLineEdit::textOnEditingFinished),this,&InfoTreeWidgetController::setCitationJournalNumber);
  QObject::connect(_infoCitationForm->publicationDateEdit, &QDateEdit::dateChanged,this,&InfoTreeWidgetController::setCitationPublicationDate);
  QObject::connect(_infoCitationForm->publicationDOILineEdit,static_cast<void (CustomLineEdit::*)(const QString&)>(&CustomLineEdit::textOnEditingFinished),this,&InfoTreeWidgetController::setCitationDOI);
  QObject::connect(_infoCitationForm->publicationDatabaseCodeLineEdit,static_cast<void (CustomLineEdit::*)(const QString&)>(&CustomLineEdit::textOnEditingFinished),this,&InfoTreeWidgetController::setCitationDatabaseCodes);


  // Expanding
  //=========================================================================
  pushButtonCreator->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  pushButtonCreation->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  pushButtonChemical->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  pushButtonCitation->setFocusPolicy(Qt::FocusPolicy::NoFocus);

#if (QT_VERSION >= QT_VERSION_CHECK(5,3,0))
  _infoCreationForm->experimentalDetailsTextEdit->setPlaceholderText(QApplication::translate("InfoCreationMainWidget", "Details", 0));
#endif

  QObject::connect(pushButtonCreator, &QPushButton::clicked, this, &InfoTreeWidgetController::expandCreatorItem);
  QObject::connect(pushButtonCreation, &QPushButton::clicked, this, &InfoTreeWidgetController::expandCreationItem);
  QObject::connect(pushButtonChemical, &QPushButton::clicked, this, &InfoTreeWidgetController::expandChemicalItem);
  QObject::connect(pushButtonCitation, &QPushButton::clicked, this, &InfoTreeWidgetController::expandCitationItem);
}

void InfoTreeWidgetController::setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode)
{
  _projectTreeNode = projectTreeNode;
  _projectStructure = nullptr;
  _iraspa_structures = std::vector<std::shared_ptr<iRASPAStructure>>{};

  if (projectTreeNode)
  {
    if(std::shared_ptr<iRASPAProject> iraspaProject = projectTreeNode->representedObject())
    {
      if(std::shared_ptr<Project> project = iraspaProject->project())
      {
        if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
        {
          _projectStructure = projectStructure;
          _iraspa_structures = projectStructure->sceneList()->flattenedAllIRASPAStructures();
        }
      }
    }
  }

  reloadData();
}

void InfoTreeWidgetController::setFlattenedSelectedFrames(std::vector<std::shared_ptr<iRASPAStructure>> iraspa_structures)
{
  _iraspa_structures = iraspa_structures;
  reloadData();
}

void InfoTreeWidgetController::expandCreatorItem()
{
  QModelIndex index = indexFromItem(topLevelItem(0),0);
  if(this->isExpanded(index))
  {
    this->collapse(index);
    pushButtonCreator->setIcon(QIcon(":/iraspa/collapsed.png"));
  }
  else
  {
    this->expand(index);
    pushButtonCreator->setIcon(QIcon(":/iraspa/expanded.png"));
  }
}

void InfoTreeWidgetController::expandCreationItem()
{
  QModelIndex index = indexFromItem(topLevelItem(1),0);
  if(this->isExpanded(index))
  {
    this->collapse(index);
    pushButtonCreation->setIcon(QIcon(":/iraspa/collapsed.png"));
  }
  else
  {
    this->expand(index);
    pushButtonCreation->setIcon(QIcon(":/iraspa/expanded.png"));
    reloadCreationData();
    update();
  }
}

void InfoTreeWidgetController::expandChemicalItem()
{
  QModelIndex index = indexFromItem(topLevelItem(2),0);
  if(this->isExpanded(index))
  {
    this->collapse(index);
    pushButtonChemical->setIcon(QIcon(":/iraspa/collapsed.png"));
  }
  else
  {
    this->expand(index);
    pushButtonChemical->setIcon(QIcon(":/iraspa/expanded.png"));
  }
}

void InfoTreeWidgetController::expandCitationItem()
{
  QModelIndex index = indexFromItem(topLevelItem(3),0);
  if(this->isExpanded(index))
  {
    this->collapse(index);
    pushButtonCitation->setIcon(QIcon(":/iraspa/collapsed.png"));
  }
  else
  {
    this->expand(index);
    pushButtonCitation->setIcon(QIcon(":/iraspa/expanded.png"));
  }
}

void InfoTreeWidgetController::resetData()
{
  reloadData();
}

void InfoTreeWidgetController::reloadData()
{
  reloadCreatorData();
  reloadCreationData();
  reloadChemicalData();
  reloadCitationData();

  update();
}

// reload Creator properties
//========================================================================================================================================

void InfoTreeWidgetController::reloadCreatorData()
{
  reloadAuthorFirstName();
  reloadAuthorMiddleName();
  reloadAuthorLastName();
  reloadAuthorOrchidID();
  reloadAuthorResearcherID();
  reloadAuthorAffiliationUniversityName();
  reloadAuthorAffiliationFacultyName();
  reloadAuthorAffiliationInstituteName();
  reloadAuthorAffiliationCityName();
  reloadAuthorAffiliationCountryName();
}

void InfoTreeWidgetController::reloadAuthorFirstName()
{
  _infoCreatorForm->firstNameLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreatorForm->firstNameLineEdit->setEnabled(true);
      _infoCreatorForm->firstNameLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = authorFirstName())
      {
        whileBlocking(_infoCreatorForm->firstNameLineEdit)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreatorForm->firstNameLineEdit)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadAuthorMiddleName()
{
  _infoCreatorForm->middleNameLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreatorForm->middleNameLineEdit->setEnabled(true);
      _infoCreatorForm->middleNameLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = authorMiddleName())
      {
        whileBlocking(_infoCreatorForm->middleNameLineEdit)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreatorForm->middleNameLineEdit)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadAuthorLastName()
{
  _infoCreatorForm->lastNameLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreatorForm->lastNameLineEdit->setEnabled(true);
      _infoCreatorForm->lastNameLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = authorLastName())
      {
        whileBlocking(_infoCreatorForm->lastNameLineEdit)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreatorForm->lastNameLineEdit)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadAuthorOrchidID()
{
  _infoCreatorForm->orchidLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreatorForm->orchidLineEdit->setEnabled(true);
      _infoCreatorForm->orchidLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = authorOrchidID())
      {
        whileBlocking(_infoCreatorForm->orchidLineEdit)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreatorForm->orchidLineEdit)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadAuthorResearcherID()
{
  _infoCreatorForm->researcherIDLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreatorForm->researcherIDLineEdit->setEnabled(true);
      _infoCreatorForm->researcherIDLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = authorResearcherID())
      {
        whileBlocking(_infoCreatorForm->researcherIDLineEdit)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreatorForm->researcherIDLineEdit)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadAuthorAffiliationUniversityName()
{
  _infoCreatorForm->universityNameLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreatorForm->universityNameLineEdit->setEnabled(true);
      _infoCreatorForm->universityNameLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = authorAffiliationUniversityName())
      {
        whileBlocking(_infoCreatorForm->universityNameLineEdit)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreatorForm->universityNameLineEdit)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadAuthorAffiliationFacultyName()
{
  _infoCreatorForm->facultyNameLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreatorForm->facultyNameLineEdit->setEnabled(true);
      _infoCreatorForm->facultyNameLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = authorAffiliationFacultyName())
      {
        whileBlocking(_infoCreatorForm->facultyNameLineEdit)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreatorForm->facultyNameLineEdit)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadAuthorAffiliationInstituteName()
{
  _infoCreatorForm->instituteNameLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreatorForm->instituteNameLineEdit->setEnabled(true);
      _infoCreatorForm->instituteNameLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = authorAffiliationInstituteName())
      {
        whileBlocking(_infoCreatorForm->instituteNameLineEdit)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreatorForm->instituteNameLineEdit)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadAuthorAffiliationCityName()
{
  _infoCreatorForm->cityNameLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreatorForm->cityNameLineEdit->setEnabled(true);
      _infoCreatorForm->cityNameLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = authorAffiliationCityName())
      {
        whileBlocking(_infoCreatorForm->cityNameLineEdit)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreatorForm->cityNameLineEdit)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadAuthorAffiliationCountryName()
{
  _infoCreatorForm->countryComboBox->setDisabled(true);

  if(!_iraspa_structures.empty())
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreatorForm->countryComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<QString> type = authorAffiliationCountryName())
      {
        if(int index = _infoCreatorForm->countryComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_infoCreatorForm->countryComboBox)->removeItem(index);
        }
        whileBlocking(_infoCreatorForm->countryComboBox)->setCurrentText(*type);
      }
      else
      {
        if(int index = _infoCreatorForm->countryComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_infoCreatorForm->countryComboBox)->addItem("Multiple values");
        }
        whileBlocking(_infoCreatorForm->countryComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

std::optional<QString> InfoTreeWidgetController::authorFirstName()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->authorFirstName();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setAuthorFirstName(QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAuthorFirstName(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::authorMiddleName()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->authorMiddleName();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setAuthorMiddleName(QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAuthorMiddleName(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::authorLastName()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->authorLastName();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setAuthorLastName(QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAuthorLastName(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::authorOrchidID()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->authorOrchidID();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setAuthorOrchidID(QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAuthorOrchidID(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::authorResearcherID()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->authorResearcherID();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setAuthorResearcherID(QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAuthorResearcherID(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::authorAffiliationUniversityName()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->authorAffiliationUniversityName();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setAuthorAffiliationUniversityName(QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAuthorAffiliationUniversityName(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::authorAffiliationFacultyName()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->authorAffiliationFacultyName();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setAuthorAffiliationFacultyName(QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAuthorAffiliationFacultyName(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::authorAffiliationInstituteName()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->authorAffiliationInstituteName();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setAuthorAffiliationInstituteName(QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAuthorAffiliationInstituteName(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::authorAffiliationCityName()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->authorAffiliationCityName();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setAuthorAffiliationCityName(QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAuthorAffiliationCityName(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::authorAffiliationCountryName()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->authorAffiliationCountryName();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setAuthorAffiliationCountryName(const QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setAuthorAffiliationCountryName(name);
  }

  _mainWindow->documentWasModified();
}

// reload Creation properties
//========================================================================================================================================

void InfoTreeWidgetController::reloadCreationData()
{
  reloadCreationDate();
  reloadCreationTemperature();
  reloadCreationTemperatureUnit();
  reloadCreationPressure();
  reloadCreationPressureUnit();
  reloadCreationMethod();

  reloadCreationRelaxUnitCell();
  reloadCreationAtomicPositionsSoftwarePackage();
  reloadCreationAtomicPositionsAlgorithm();
  reloadCreationAtomicPositionsEigenvalues();
  reloadCreationAtomicPositionsForceField();
  reloadCreationAtomicPositionsForceFieldDetails();

  reloadCreationAtomicChargesSoftwarePackage();
  reloadCreationAtomicChargesAlgorithm();
  reloadCreationAtomicChargesForceField();
  reloadCreationAtomicChargesForceFieldDetails();

  reloadExperimentalRadiationType();
  reloadExperimentalWaveLength();
  reloadExperimentalMeasuermentThetaMin();
  reloadExperimentalMeasuermentThetaMax();
  reloadExperimentalMeasuermentHMin();
  reloadExperimentalMeasuermentHMax();
  reloadExperimentalMeasuermentKMin();
  reloadExperimentalMeasuermentKMax();
  reloadExperimentalMeasuermentLMin();
  reloadExperimentalMeasuermentLMax();
  reloadExperimentalNumberOfReflections();
  reloadExperimentalSoftware();
  reloadExperimentalDetails();
  reloadExperimentalGoodnessOfFit();
  reloadExperimentalFinalIndices();
  reloadExperimentalRIndicest();
}

void InfoTreeWidgetController::reloadCreationDate()
{
  _infoCreationForm->creationDateEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->creationDateEdit->setEnabled(true);
      _infoCreationForm->creationDateEdit->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QDate> value = creationDate())
      {
        whileBlocking(_infoCreationForm->creationDateEdit)->setDate(*value);
      }
      else
      {
        whileBlocking(_infoCreationForm->creationDateEdit)->setDate(QDate());
      }
    }
  }
}

void InfoTreeWidgetController::reloadCreationTemperature()
{
  _infoCreationForm->temperatureLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->temperatureLineEdit->setEnabled(true);
      _infoCreationForm->temperatureLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = creationTemperature())
      {
         whileBlocking(_infoCreationForm->temperatureLineEdit)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreationForm->temperatureLineEdit)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadCreationTemperatureUnit()
{
  _infoCreationForm->temperatureComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->temperatureComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<Structure::TemperatureScale> value = creationTemperatureScale())
      {
        if(int index = _infoCreationForm->temperatureComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_infoCreationForm->temperatureComboBox)->removeItem(index);
        }
        whileBlocking(_infoCreationForm->temperatureComboBox)->setCurrentIndex(int(*value));
      }
      else
      {
        if(int index = _infoCreationForm->temperatureComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_infoCreationForm->temperatureComboBox)->addItem("Multiple values");
        }
        whileBlocking(_infoCreationForm->temperatureComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

void InfoTreeWidgetController::reloadCreationPressure()
{
  _infoCreationForm->pressureLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->pressureLineEdit->setEnabled(true);
      _infoCreationForm->pressureLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = creationPressure())
      {
        whileBlocking(_infoCreationForm->pressureLineEdit)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreationForm->pressureLineEdit)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadCreationPressureUnit()
{
  _infoCreationForm->pressureComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->pressureComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<Structure::PressureScale> value = creationPressureScale())
      {
        if(int index = _infoCreationForm->pressureComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_infoCreationForm->pressureComboBox)->removeItem(index);
        }
        whileBlocking(_infoCreationForm->pressureComboBox)->setCurrentIndex(int(*value));
      }
      else
      {
        if(int index = _infoCreationForm->pressureComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_infoCreationForm->pressureComboBox)->addItem("Multiple values");
        }
        whileBlocking(_infoCreationForm->pressureComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

void InfoTreeWidgetController::reloadCreationMethod()
{
  _infoCreationForm->methodComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->methodComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<Structure::CreationMethod> value = creationMethod())
      {
        if(int index = _infoCreationForm->methodComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_infoCreationForm->methodComboBox)->removeItem(index);
        }
        whileBlocking(_infoCreationForm->methodComboBox)->setCurrentIndex(int(*value));
      }
      else
      {
        if(int index = _infoCreationForm->methodComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_infoCreationForm->methodComboBox)->addItem("Multiple values");
        }
        whileBlocking(_infoCreationForm->methodComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

void InfoTreeWidgetController::reloadCreationRelaxUnitCell()
{
  _infoCreationForm->relaxUnitCellComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->relaxUnitCellComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<Structure::UnitCellRelaxationMethod> value = creationRelaxUnitCell())
      {
        if(int index = _infoCreationForm->relaxUnitCellComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_infoCreationForm->relaxUnitCellComboBox)->removeItem(index);
        }
        whileBlocking(_infoCreationForm->relaxUnitCellComboBox)->setCurrentIndex(int(*value));
      }
      else
      {
        if(int index = _infoCreationForm->relaxUnitCellComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_infoCreationForm->relaxUnitCellComboBox)->addItem("Multiple values");
        }
        whileBlocking(_infoCreationForm->relaxUnitCellComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

void InfoTreeWidgetController::reloadCreationAtomicPositionsSoftwarePackage()
{
  _infoCreationForm->positionsSoftwarePackageComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->positionsSoftwarePackageComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<QString> value = creationAtomicPositionsSoftwarePackage())
      {
        if(int index = _infoCreationForm->positionsSoftwarePackageComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_infoCreationForm->positionsSoftwarePackageComboBox)->removeItem(index);
        }
        whileBlocking(_infoCreationForm->positionsSoftwarePackageComboBox)->setCurrentText(*value);
      }
      else
      {
        if(int index = _infoCreationForm->positionsSoftwarePackageComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_infoCreationForm->positionsSoftwarePackageComboBox)->addItem("Multiple values");
        }
        whileBlocking(_infoCreationForm->positionsSoftwarePackageComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

void InfoTreeWidgetController::reloadCreationAtomicPositionsAlgorithm()
{
  _infoCreationForm->positionsAlgorithmComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->positionsAlgorithmComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<Structure::IonsRelaxationAlgorithm> value = creationAtomicPositionsAlgorithm())
      {
        if(int index = _infoCreationForm->positionsAlgorithmComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_infoCreationForm->positionsAlgorithmComboBox)->removeItem(index);
        }
        whileBlocking(_infoCreationForm->positionsAlgorithmComboBox)->setCurrentIndex(int(*value));
      }
      else
      {
        if(int index = _infoCreationForm->positionsAlgorithmComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_infoCreationForm->positionsAlgorithmComboBox)->addItem("Multiple values");
        }
        whileBlocking(_infoCreationForm->positionsAlgorithmComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

void InfoTreeWidgetController::reloadCreationAtomicPositionsEigenvalues()
{
  _infoCreationForm->positionsEigenvaluesComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->positionsEigenvaluesComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<Structure::IonsRelaxationCheck> value = creationAtomicPositionsEigenvalues())
      {
        if(int index = _infoCreationForm->positionsEigenvaluesComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_infoCreationForm->positionsEigenvaluesComboBox)->removeItem(index);
        }
        whileBlocking(_infoCreationForm->positionsEigenvaluesComboBox)->setCurrentIndex(int(*value));
      }
      else
      {
        if(int index = _infoCreationForm->positionsEigenvaluesComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_infoCreationForm->positionsEigenvaluesComboBox)->addItem("Multiple values");
        }
        whileBlocking(_infoCreationForm->positionsEigenvaluesComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

void InfoTreeWidgetController::reloadCreationAtomicPositionsForceField()
{
  _infoCreationForm->positionsForceFieldComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->positionsForceFieldComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<QString> value = creationAtomicPositionsForceField())
      {
        if(int index = _infoCreationForm->positionsForceFieldComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_infoCreationForm->positionsForceFieldComboBox)->removeItem(index);
        }
        whileBlocking(_infoCreationForm->positionsForceFieldComboBox)->setCurrentText(*value);
      }
      else
      {
        if(int index = _infoCreationForm->positionsForceFieldComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_infoCreationForm->positionsForceFieldComboBox)->addItem("Multiple values");
        }
        whileBlocking(_infoCreationForm->positionsForceFieldComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

void InfoTreeWidgetController::reloadCreationAtomicPositionsForceFieldDetails()
{
  _infoCreationForm->positionsForceFieldDetailsComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->positionsForceFieldDetailsComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<QString> value = creationAtomicPositionsForceFieldDetails())
      {
        if(int index = _infoCreationForm->positionsForceFieldDetailsComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_infoCreationForm->positionsForceFieldDetailsComboBox)->removeItem(index);
        }
        whileBlocking(_infoCreationForm->positionsForceFieldDetailsComboBox)->setCurrentText(*value);
      }
      else
      {
        if(int index = _infoCreationForm->positionsForceFieldDetailsComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_infoCreationForm->positionsForceFieldDetailsComboBox)->addItem("Multiple values");
        }
        whileBlocking(_infoCreationForm->positionsForceFieldDetailsComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

void InfoTreeWidgetController::reloadCreationAtomicChargesSoftwarePackage()
{
  _infoCreationForm->chargeSoftwarePackageComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->chargeSoftwarePackageComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<QString> value = creationAtomicChargesSoftwarePackage())
      {
        if(int index = _infoCreationForm->chargeSoftwarePackageComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_infoCreationForm->chargeSoftwarePackageComboBox)->removeItem(index);
        }
        whileBlocking(_infoCreationForm->chargeSoftwarePackageComboBox)->setCurrentText(*value);
      }
      else
      {
        if(int index = _infoCreationForm->chargeSoftwarePackageComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_infoCreationForm->chargeSoftwarePackageComboBox)->addItem("Multiple values");
        }
        whileBlocking(_infoCreationForm->chargeSoftwarePackageComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

void InfoTreeWidgetController::reloadCreationAtomicChargesAlgorithm()
{
  _infoCreationForm->chargeAlgorithmComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->chargeAlgorithmComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<QString> value = creationAtomicChargesAlgorithm())
      {
        if(int index = _infoCreationForm->chargeAlgorithmComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_infoCreationForm->chargeAlgorithmComboBox)->removeItem(index);
        }
        whileBlocking(_infoCreationForm->chargeAlgorithmComboBox)->setCurrentText(*value);
      }
      else
      {
        if(int index = _infoCreationForm->chargeAlgorithmComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_infoCreationForm->chargeAlgorithmComboBox)->addItem("Multiple values");
        }
        whileBlocking(_infoCreationForm->chargeAlgorithmComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}


void InfoTreeWidgetController::reloadCreationAtomicChargesForceField()
{
  _infoCreationForm->chargeForceFieldComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->chargeForceFieldComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<QString> value = creationAtomicChargesForceField())
      {
        if(int index = _infoCreationForm->chargeForceFieldComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_infoCreationForm->chargeForceFieldComboBox)->removeItem(index);
        }
        whileBlocking(_infoCreationForm->chargeForceFieldComboBox)->setCurrentText(*value);
      }
      else
      {
        if(int index = _infoCreationForm->chargeForceFieldComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_infoCreationForm->chargeForceFieldComboBox)->addItem("Multiple values");
        }
        whileBlocking(_infoCreationForm->chargeForceFieldComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

void InfoTreeWidgetController::reloadCreationAtomicChargesForceFieldDetails()
{
  _infoCreationForm->chargeForceFieldDetailsComboBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->chargeForceFieldDetailsComboBox->setEnabled(_projectTreeNode->isEditable());

      if (std::optional<QString> value = creationAtomicChargesForceFieldDetails())
      {
        if(int index = _infoCreationForm->chargeForceFieldDetailsComboBox->findText("Multiple values"); index>=0)
        {
          whileBlocking(_infoCreationForm->chargeForceFieldDetailsComboBox)->removeItem(index);
        }
        whileBlocking(_infoCreationForm->chargeForceFieldDetailsComboBox)->setCurrentText(*value);
      }
      else
      {
        if(int index = _infoCreationForm->chargeForceFieldDetailsComboBox->findText("Multiple values"); index<0)
        {
          whileBlocking(_infoCreationForm->chargeForceFieldDetailsComboBox)->addItem("Multiple values");
        }
        whileBlocking(_infoCreationForm->chargeForceFieldDetailsComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

std::optional<QDate> InfoTreeWidgetController::creationDate()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QDate> set = std::set<QDate>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QDate value = iraspa_structure->structure()->creationDate();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationDate(const QDate &date)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setCreationDate(date);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::creationTemperature()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->creationTemperature();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationTemperature(QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setCreationTemperature(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<Structure::TemperatureScale> InfoTreeWidgetController::creationTemperatureScale()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<Structure::TemperatureScale> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    Structure::TemperatureScale value = iraspa_structure->structure()->creationTemperatureScale();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationTemperatureScale(int value)
{
  if(value>=0 && value<int(Structure::TemperatureScale::multiple_values))
  {
    for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setCreationTemperatureScale(Structure::TemperatureScale(value));
    }

    _mainWindow->documentWasModified();
  }
}

std::optional<QString> InfoTreeWidgetController::creationPressure()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->creationPressure();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationPressure(QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setCreationPressure(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<Structure::PressureScale> InfoTreeWidgetController::creationPressureScale()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<Structure::PressureScale> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    Structure::PressureScale value = iraspa_structure->structure()->creationPressureScale();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationPressureScale(int value)
{
  if(value>=0 && value<int(Structure::PressureScale::multiple_values))
  {
    for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setCreationPressureScale(Structure::PressureScale(value));
    }

    _mainWindow->documentWasModified();
  }
}

std::optional<Structure::CreationMethod> InfoTreeWidgetController::creationMethod()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<Structure::CreationMethod> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    Structure::CreationMethod value = iraspa_structure->structure()->creationMethod();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationMethod(int value)
{
  if(value>=0 && value<int(Structure::CreationMethod::multiple_values))
  {
    for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setCreationMethod(Structure::CreationMethod(value));
    }

    _mainWindow->documentWasModified();
  }
}

std::optional<Structure::UnitCellRelaxationMethod> InfoTreeWidgetController::creationRelaxUnitCell()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<Structure::UnitCellRelaxationMethod> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    Structure::UnitCellRelaxationMethod value = iraspa_structure->structure()->creationUnitCellRelaxationMethod();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationRelaxUnitCell(int value)
{
  if(value>=0 && value<int(Structure::UnitCellRelaxationMethod::multiple_values))
  {
    for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setCreationUnitCellRelaxationMethod(Structure::UnitCellRelaxationMethod(value));
    }

    _mainWindow->documentWasModified();
  }
}

std::optional<QString> InfoTreeWidgetController::creationAtomicPositionsSoftwarePackage()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->creationAtomicPositionsSoftwarePackage();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationAtomicPositionsSoftwarePackage(QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setCreationAtomicPositionsSoftwarePackage(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<Structure::IonsRelaxationAlgorithm> InfoTreeWidgetController::creationAtomicPositionsAlgorithm()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<Structure::IonsRelaxationAlgorithm> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    Structure::IonsRelaxationAlgorithm value = iraspa_structure->structure()->creationAtomicPositionsIonsRelaxationAlgorithm();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationAtomicPositionsAlgorithm(int value)
{
  if(value>=0 && value<int(Structure::IonsRelaxationAlgorithm::multiple_values))
  {
    for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setCreationAtomicPositionsIonsRelaxationAlgorithm(Structure::IonsRelaxationAlgorithm(value));
    }

    _mainWindow->documentWasModified();
  }
}

std::optional<Structure::IonsRelaxationCheck> InfoTreeWidgetController::creationAtomicPositionsEigenvalues()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<Structure::IonsRelaxationCheck> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    Structure::IonsRelaxationCheck value = iraspa_structure->structure()->creationAtomicPositionsIonsRelaxationCheck();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationAtomicPositionsEigenvalues(int value)
{
  if(value>=0 && value<int(Structure::IonsRelaxationCheck::multiple_values))
  {
    for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
    {
      iraspa_structure->structure()->setCreationAtomicPositionsIonsRelaxationCheck(Structure::IonsRelaxationCheck(value));
    }

    _mainWindow->documentWasModified();
  }
}

std::optional<QString> InfoTreeWidgetController::creationAtomicPositionsForceField()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->creationAtomicPositionsForcefield();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationAtomicPositionsForceField(QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setCreationAtomicPositionsForcefield(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::creationAtomicPositionsForceFieldDetails()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->creationAtomicPositionsForcefieldDetails();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationAtomicPositionsForceFieldDetails(QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setCreationAtomicPositionsForcefieldDetails(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::creationAtomicChargesSoftwarePackage()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->creationAtomicChargesSoftwarePackage();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationAtomicChargesSoftwarePackage(QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setCreationAtomicChargesSoftwarePackage(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::creationAtomicChargesAlgorithm()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->creationAtomicChargesAlgorithms();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationAtomicChargesAlgorithm(QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setCreationAtomicChargesAlgorithms(name);
  }

  _mainWindow->documentWasModified();
}


std::optional<QString> InfoTreeWidgetController::creationAtomicChargesForceField()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->creationAtomicChargesForcefield();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationAtomicChargesForceField(QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setCreationAtomicChargesForcefield(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::creationAtomicChargesForceFieldDetails()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->creationAtomicChargesForcefieldDetails();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationAtomicChargesForceFieldDetails(QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setCreationAtomicChargesForcefieldDetails(name);
  }

  _mainWindow->documentWasModified();
}


void InfoTreeWidgetController::reloadExperimentalRadiationType()
{
  _infoCreationForm->experimentalRadiationTypeLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->experimentalRadiationTypeLineEdit->setEnabled(true);
      _infoCreationForm->experimentalRadiationTypeLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = creationExperimentalRadiationType())
      {
        whileBlocking(_infoCreationForm->experimentalRadiationTypeLineEdit)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreationForm->experimentalRadiationTypeLineEdit)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadExperimentalWaveLength()
{
  _infoCreationForm->experimentalRadiationWavelengthDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->experimentalRadiationWavelengthDoubleSpinBox->setEnabled(true);
      _infoCreationForm->experimentalRadiationWavelengthDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = creationExperimentalWaveLength())
      {
        whileBlocking(_infoCreationForm->experimentalRadiationWavelengthDoubleSpinBox)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreationForm->experimentalRadiationWavelengthDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadExperimentalMeasuermentThetaMin()
{
  _infoCreationForm->experimentalMeasurementThetaMinDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->experimentalMeasurementThetaMinDoubleSpinBox->setEnabled(true);
      _infoCreationForm->experimentalMeasurementThetaMinDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = creationExperimentalMeasuermentThetaMin())
      {
        whileBlocking(_infoCreationForm->experimentalMeasurementThetaMinDoubleSpinBox)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreationForm->experimentalMeasurementThetaMinDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadExperimentalMeasuermentThetaMax()
{
  _infoCreationForm->experimentalMeasurementThetaMaxDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->experimentalMeasurementThetaMaxDoubleSpinBox->setEnabled(true);
      _infoCreationForm->experimentalMeasurementThetaMaxDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = creationExperimentalMeasuermentThetaMax())
      {
        whileBlocking(_infoCreationForm->experimentalMeasurementThetaMaxDoubleSpinBox)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreationForm->experimentalMeasurementThetaMaxDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadExperimentalMeasuermentHMin()
{
  _infoCreationForm->experimentalhminDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->experimentalhminDoubleSpinBox->setEnabled(true);
      _infoCreationForm->experimentalhminDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = creationExperimentalMeasuermentHMin())
      {
        whileBlocking(_infoCreationForm->experimentalhminDoubleSpinBox)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreationForm->experimentalhminDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadExperimentalMeasuermentHMax()
{
  _infoCreationForm->experimentalhmaxDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->experimentalhmaxDoubleSpinBox->setEnabled(true);
      _infoCreationForm->experimentalhmaxDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = creationExperimentalMeasuermentHMax())
      {
        whileBlocking(_infoCreationForm->experimentalhmaxDoubleSpinBox)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreationForm->experimentalhmaxDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadExperimentalMeasuermentKMin()
{
  _infoCreationForm->experimentalkminDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->experimentalkminDoubleSpinBox->setEnabled(true);
      _infoCreationForm->experimentalkminDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = creationExperimentalMeasuermentKMin())
      {
        whileBlocking(_infoCreationForm->experimentalkminDoubleSpinBox)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreationForm->experimentalkminDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadExperimentalMeasuermentKMax()
{
  _infoCreationForm->experimentalkmaxDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->experimentalkmaxDoubleSpinBox->setEnabled(true);
      _infoCreationForm->experimentalkmaxDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = creationExperimentalMeasuermentKMax())
      {
        whileBlocking(_infoCreationForm->experimentalkmaxDoubleSpinBox)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreationForm->experimentalkmaxDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadExperimentalMeasuermentLMin()
{
  _infoCreationForm->experimentallminDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->experimentallminDoubleSpinBox->setEnabled(true);
      _infoCreationForm->experimentallminDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = creationExperimentalMeasuermentLMin())
      {
        whileBlocking(_infoCreationForm->experimentallminDoubleSpinBox)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreationForm->experimentallminDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadExperimentalMeasuermentLMax()
{
  _infoCreationForm->experimentallmaxDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->experimentallmaxDoubleSpinBox->setEnabled(true);
      _infoCreationForm->experimentallmaxDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = creationExperimentalMeasuermentLMax())
      {
        whileBlocking(_infoCreationForm->experimentallmaxDoubleSpinBox)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreationForm->experimentallmaxDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadExperimentalNumberOfReflections()
{
  _infoCreationForm->experimentalNumberOfReflectionsSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->experimentalNumberOfReflectionsSpinBox->setEnabled(true);
      _infoCreationForm->experimentalNumberOfReflectionsSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = creationExperimentalNumberOfReflections())
      {
        whileBlocking(_infoCreationForm->experimentalNumberOfReflectionsSpinBox)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreationForm->experimentalNumberOfReflectionsSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadExperimentalSoftware()
{
  _infoCreationForm->experimentalSoftwareLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->experimentalSoftwareLineEdit->setEnabled(true);
      _infoCreationForm->experimentalSoftwareLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = creationExperimentalSoftware())
      {
        whileBlocking(_infoCreationForm->experimentalSoftwareLineEdit)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreationForm->experimentalSoftwareLineEdit)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadExperimentalDetails()
{
  _infoCreationForm->experimentalDetailsTextEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->experimentalDetailsTextEdit->setEnabled(true);
      _infoCreationForm->experimentalDetailsTextEdit->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = creationExperimentalDetails())
      {
        whileBlocking(_infoCreationForm->experimentalDetailsTextEdit)->setPlainText(*value);
      }
      else
      {
        whileBlocking(_infoCreationForm->experimentalDetailsTextEdit)->setPlainText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadExperimentalGoodnessOfFit()
{
  _infoCreationForm->experimentalGoodnessOfFitDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->experimentalGoodnessOfFitDoubleSpinBox->setEnabled(true);
      _infoCreationForm->experimentalGoodnessOfFitDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = creationExperimentalGoodnessOfFit())
      {
        whileBlocking(_infoCreationForm->experimentalGoodnessOfFitDoubleSpinBox)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreationForm->experimentalGoodnessOfFitDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadExperimentalFinalIndices()
{
  _infoCreationForm->experimentalFinalIndicesDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->experimentalFinalIndicesDoubleSpinBox->setEnabled(true);
      _infoCreationForm->experimentalFinalIndicesDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = creationExperimentalFinalIndices())
      {
        whileBlocking(_infoCreationForm->experimentalFinalIndicesDoubleSpinBox)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreationForm->experimentalFinalIndicesDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadExperimentalRIndicest()
{
  _infoCreationForm->experimentalRIndicesDoubleSpinBox->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoCreationForm->experimentalRIndicesDoubleSpinBox->setEnabled(true);
      _infoCreationForm->experimentalRIndicesDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = creationExperimentalRIndicest())
      {
        whileBlocking(_infoCreationForm->experimentalRIndicesDoubleSpinBox)->setText(*value);
      }
      else
      {
        whileBlocking(_infoCreationForm->experimentalRIndicesDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}


std::optional<QString> InfoTreeWidgetController::creationExperimentalRadiationType()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->experimentalMeasurementRadiation();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationExperimentalRadiationType(const QString& type)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setExperimentalMeasurementRadiation(type);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::creationExperimentalWaveLength()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->experimentalMeasurementWaveLength();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationExperimentalWaveLength(const QString& wavelength)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setExperimentalMeasurementWaveLength(wavelength);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::creationExperimentalMeasuermentThetaMin()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->experimentalMeasurementThetaMin();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}


void InfoTreeWidgetController::setCreationExperimentalMeasuermentThetaMin(const QString& thetamin)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setExperimentalMeasurementThetaMin(thetamin);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::creationExperimentalMeasuermentThetaMax()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->experimentalMeasurementThetaMax();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationExperimentalMeasuermentThetaMax(const QString& thetamax)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setExperimentalMeasurementThetaMax(thetamax);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::creationExperimentalMeasuermentHMin()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->experimentalMeasurementIndexLimitsHmin();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationExperimentalMeasuermentHMin(const QString& hmin)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setExperimentalMeasurementIndexLimitsHmin(hmin);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::creationExperimentalMeasuermentHMax()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->experimentalMeasurementIndexLimitsHmax();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationExperimentalMeasuermentHMax(const QString& hmax)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setExperimentalMeasurementIndexLimitsHmax(hmax);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::creationExperimentalMeasuermentKMin()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->experimentalMeasurementIndexLimitsKmin();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationExperimentalMeasuermentKMin(const QString& kmin)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setExperimentalMeasurementIndexLimitsKmin(kmin);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::creationExperimentalMeasuermentKMax()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->experimentalMeasurementIndexLimitsKmax();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationExperimentalMeasuermentKMax(const QString& kmax)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setExperimentalMeasurementIndexLimitsKmax(kmax);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::creationExperimentalMeasuermentLMin()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->experimentalMeasurementIndexLimitsLmin();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationExperimentalMeasuermentLMin(const QString& lmin)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setExperimentalMeasurementIndexLimitsLmin(lmin);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::creationExperimentalMeasuermentLMax()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->experimentalMeasurementIndexLimitsLmax();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationExperimentalMeasuermentLMax(const QString& lmax)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setExperimentalMeasurementIndexLimitsLmax(lmax);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::creationExperimentalNumberOfReflections()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->experimentalMeasurementNumberOfSymmetryIndependentReflections();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationExperimentalNumberOfReflections(const QString& reflections)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setExperimentalMeasurementNumberOfSymmetryIndependentReflections(reflections);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::creationExperimentalSoftware()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->experimentalMeasurementSoftware();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationExperimentalSoftware(const QString& software)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setExperimentalMeasurementSoftware(software);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::creationExperimentalDetails()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->experimentalMeasurementRefinementDetails();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationExperimentalDetails(const QString& details)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setExperimentalMeasurementRefinementDetails(details);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::creationExperimentalGoodnessOfFit()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->experimentalMeasurementGoodnessOfFit();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationExperimentalGoodnessOfFit(const QString& goodness)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setExperimentalMeasurementGoodnessOfFit(goodness);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::creationExperimentalFinalIndices()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->experimentalMeasurementRFactorGt();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationExperimentalFinalIndices(const QString& indices)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setExperimentalMeasurementRFactorGt(indices);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::creationExperimentalRIndicest()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->experimentalMeasurementRFactorAll();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCreationExperimentalRIndicest(const QString& indices)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setExperimentalMeasurementRFactorAll(indices);
  }

  _mainWindow->documentWasModified();
}

// reload Chemical properties
//========================================================================================================================================

void InfoTreeWidgetController::reloadChemicalData()
{
  reloadChemicalFormulaMoiety();
  reloadChemicalFormulaSum();
  reloadChemicalNameSystematic();
}

void InfoTreeWidgetController::reloadChemicalFormulaMoiety()
{
  _infoChemicalForm->moietyLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoChemicalForm->moietyLineEdit->setEnabled(true);
      _infoChemicalForm->moietyLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = chemicalFormulaMoiety())
      {
        whileBlocking(_infoChemicalForm->moietyLineEdit)->setText(*value);
      }
      else
      {
        whileBlocking(_infoChemicalForm->moietyLineEdit)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadChemicalFormulaSum()
{
  _infoChemicalForm->sumLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoChemicalForm->sumLineEdit->setEnabled(true);
      _infoChemicalForm->sumLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = chemicalFormulaSum())
      {
        whileBlocking(_infoChemicalForm->sumLineEdit)->setText(*value);
      }
      else
      {
        whileBlocking(_infoChemicalForm->sumLineEdit)->setText("Mult. Val.");
      }
    }
  }
}

void InfoTreeWidgetController::reloadChemicalNameSystematic()
{
  _infoChemicalForm->sytematicLineEdit->setDisabled(true);

  if(_projectTreeNode)
  {
    if(std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
    {
      _infoChemicalForm->sytematicLineEdit->setEnabled(true);
      _infoChemicalForm->sytematicLineEdit->setReadOnly(!_projectTreeNode->isEditable());

      if ( std::optional<QString> value = chemicalNameSystematic())
      {
        whileBlocking(_infoChemicalForm->sytematicLineEdit)->setText(*value);
      }
      else
      {
        whileBlocking(_infoChemicalForm->sytematicLineEdit)->setText("Mult. Val.");
      }
    }
  }
}

std::optional<QString> InfoTreeWidgetController::chemicalFormulaMoiety()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->chemicalFormulaMoiety();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setChemicalFormulaMoiety(const QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setChemicalFormulaMoiety(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::chemicalFormulaSum()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->chemicalFormulaSum();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setChemicalFormulaSum(const QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setChemicalFormulaSum(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::chemicalNameSystematic()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->chemicalNameSystematic();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setChemicalNameSystematic(const QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setChemicalNameSystematic(name);
  }

  _mainWindow->documentWasModified();
}

// reload Citation properties
//========================================================================================================================================

void InfoTreeWidgetController::reloadCitationData()
{
  reloadCitationArticleTitle();
  reloadCitationArticleAuthors();
  reloadCitationJournalVolume();
  reloadCitationJournalNumber();
  reloadCitationPublicationDate();
  reloadCitationPublicationDOI();
  reloadCitationPublicationDatabaseCodes();
}

void InfoTreeWidgetController::reloadCitationArticleTitle()
{
  _infoCitationForm->articleTitleTextEdit->setReadOnly(true);
  if(_projectTreeNode)
  {
    _infoCitationForm->articleTitleTextEdit->setReadOnly(!_projectTreeNode->isEditable());
  }

  if(!_iraspa_structures.empty())
  {
    if ( std::optional<QString> value = citationArticleTitle())
    {
      whileBlocking(_infoCitationForm->articleTitleTextEdit)->setPlainText(*value);
    }
    else
    {
      whileBlocking(_infoCitationForm->articleTitleTextEdit)->setPlainText("Mult. Val.");
    }
  }
}

void InfoTreeWidgetController::reloadCitationArticleAuthors()
{
  _infoCitationForm->articleAuthorsTextEdit->setReadOnly(true);
  if(_projectTreeNode)
  {
    _infoCitationForm->articleAuthorsTextEdit->setReadOnly(!_projectTreeNode->isEditable());
  }

  if(!_iraspa_structures.empty())
  {
    if ( std::optional<QString> value = citationArticleAuthors())
    {
      whileBlocking(_infoCitationForm->articleAuthorsTextEdit)->setPlainText(*value);
    }
    else
    {
      whileBlocking(_infoCitationForm->articleAuthorsTextEdit)->setPlainText("Mult. Val.");
    }
  }
}

void InfoTreeWidgetController::reloadCitationJournalVolume()
{
  _infoCitationForm->journalVolumeLineEdit->setReadOnly(true);
  if(_projectTreeNode)
  {
    _infoCitationForm->journalVolumeLineEdit->setReadOnly(!_projectTreeNode->isEditable());
  }

  if(!_iraspa_structures.empty())
  {
    if ( std::optional<QString> value = citationJournalVolume())
    {
      whileBlocking(_infoCitationForm->journalVolumeLineEdit)->setText(*value);
    }
    else
    {
      whileBlocking(_infoCitationForm->journalVolumeLineEdit)->setText("Mult. Val.");
    }
  }
}

void InfoTreeWidgetController::reloadCitationJournalNumber()
{
  _infoCitationForm->journalNumberLineEdit->setReadOnly(true);
  if(_projectTreeNode)
  {
    _infoCitationForm->journalNumberLineEdit->setReadOnly(!_projectTreeNode->isEditable());
  }

  if(!_iraspa_structures.empty())
  {
    if ( std::optional<QString> value = citationJournalNumber())
    {
      whileBlocking(_infoCitationForm->journalNumberLineEdit)->setText(*value);
    }
    else
    {
      whileBlocking(_infoCitationForm->journalNumberLineEdit)->setText("Mult. Val.");
    }
  }
}

void InfoTreeWidgetController::reloadCitationPublicationDate()
{
  _infoCitationForm->publicationDateEdit->setReadOnly(true);
  if(_projectTreeNode)
  {
    _infoCitationForm->publicationDateEdit->setReadOnly(!_projectTreeNode->isEditable());
  }

  if(!_iraspa_structures.empty())
  {
    if ( std::optional<QDate> value = citationPublicationDate())
    {
      whileBlocking(_infoCitationForm->publicationDateEdit)->setDate(*value);
    }
    else
    {
      whileBlocking(_infoCitationForm->publicationDateEdit)->setDate(QDate());
    }
  }
}

void InfoTreeWidgetController::reloadCitationPublicationDOI()
{
  _infoCitationForm->publicationDOILineEdit->setReadOnly(true);
  if(_projectTreeNode)
  {
    _infoCitationForm->publicationDOILineEdit->setReadOnly(!_projectTreeNode->isEditable());
  }

  if(!_iraspa_structures.empty())
  {
    if ( std::optional<QString> value = citationDOI())
    {
      whileBlocking(_infoCitationForm->publicationDOILineEdit)->setText(*value);
    }
    else
    {
      whileBlocking(_infoCitationForm->publicationDOILineEdit)->setText("Mult. Val.");
    }
  }
}

void InfoTreeWidgetController::reloadCitationPublicationDatabaseCodes()
{
  _infoCitationForm->publicationDatabaseCodeLineEdit->setReadOnly(true);
  if(_projectTreeNode)
  {
    _infoCitationForm->publicationDatabaseCodeLineEdit->setReadOnly(!_projectTreeNode->isEditable());
  }

  if(!_iraspa_structures.empty())
  {
    if ( std::optional<QString> value = citationDatabaseCodes())
    {
      whileBlocking(_infoCitationForm->publicationDatabaseCodeLineEdit)->setText(*value);
    }
    else
    {
      whileBlocking(_infoCitationForm->publicationDatabaseCodeLineEdit)->setText("Mult. Val.");
    }
  }
}

std::optional<QString> InfoTreeWidgetController::citationArticleTitle()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->citationArticleTitle();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCitationArticleTitle(const QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setCitationArticleTitle(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::citationJournalTitle()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->citationJournalTitle();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCitationJournalTitle(const QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setCitationJournalTitle(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::citationArticleAuthors()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->citationAuthors();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCitationArticleAuthors(const QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setCitationAuthors(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::InfoTreeWidgetController::citationJournalVolume()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->citationJournalVolume();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCitationJournalVolume(const QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setCitationJournalVolume(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::citationJournalNumber()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->citationJournalNumber();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCitationJournalNumber(const QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setCitationJournalNumber(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<QDate> InfoTreeWidgetController::citationPublicationDate()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
   }
  std::set<QDate> set = std::set<QDate>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QDate value = iraspa_structure->structure()->citationPublicationDate();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCitationPublicationDate(QDate date)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setCitationPublicationDate(date);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::citationDOI()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->citationDOI();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCitationDOI(const QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setCitationDOI(name);
  }

  _mainWindow->documentWasModified();
}

std::optional<QString> InfoTreeWidgetController::citationDatabaseCodes()
{
  if(_iraspa_structures.empty())
  {
    return std::nullopt;
  }
  std::set<QString> set = std::set<QString>{};
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    QString value = iraspa_structure->structure()->citationDatebaseCodes();
    set.insert(value);
  }

  if(set.size() == 1)
  {
    return *set.begin();
  }
  return std::nullopt;
}

void InfoTreeWidgetController::setCitationDatabaseCodes(const QString name)
{
  for(const std::shared_ptr<iRASPAStructure> &iraspa_structure: _iraspa_structures)
  {
    iraspa_structure->structure()->setCitationDatebaseCodes(name);
  }

  _mainWindow->documentWasModified();
}
