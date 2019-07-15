/***************************************************************************
 *   Copyright (C) 2002 by Gunnar Schmi Dt <kmouth@schmi-dt.de             *
 *             (C) 2015 by Jeremy Whiting <jpwhiting@kde.org>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include "initialphrasebookwidget.h"

// include files for Qt
#include <QDir>
#include <QLabel>
#include <QStack>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QTreeView>
#include <QVBoxLayout>

// include files for KDE
#include <QDialog>
#include <KLocalizedString>
#include <QUrl>

#include <QDebug>

InitialPhraseBookWidget::InitialPhraseBookWidget(QWidget *parent, const char *name)
    : QWizardPage(parent)
{
    setObjectName(QLatin1String(name));
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
//TODO PORT QT5     mainLayout->setSpacing(QDialog::spacingHint());
    QLabel *label = new QLabel(i18n("Please decide which phrase books you need:"), this);
    label->setObjectName(QStringLiteral("booksTitle"));
    mainLayout->addWidget(label);

    m_model = new QStandardItemModel(0, 1, this);
    m_model->setHeaderData(0, Qt::Horizontal, i18n("Book"));
    QTreeView *view = new QTreeView(this);
    view->setSortingEnabled(false);
    //books->setItemsMovable (false);
    view->setDragEnabled(false);
    view->setRootIsDecorated(true);
    view->setSelectionMode(QAbstractItemView::MultiSelection);
    view->setModel(m_model);
    mainLayout->addWidget(view);

    initStandardPhraseBooks();
    connect(m_model, &QStandardItemModel::itemChanged,
            this, &InitialPhraseBookWidget::slotItemChanged);
}

InitialPhraseBookWidget::~InitialPhraseBookWidget()
{
}

void InitialPhraseBookWidget::slotItemChanged(QStandardItem *item)
{
    if (item->hasChildren()) {
        for (int i = 0; i < item->rowCount(); ++i) {
            QStandardItem *child = item->child(i);
            child->setCheckState(item->checkState());
        }
    }
}

void InitialPhraseBookWidget::initStandardPhraseBooks()
{
    StandardBookList bookPaths = PhraseBook::standardPhraseBooks();

    QStandardItem *parent = m_model->invisibleRootItem();
    QStringList currentNamePath;
    currentNamePath << QLatin1String("");
    QStack<QStandardItem *> stack;
    StandardBookList::iterator it;
    for (it = bookPaths.begin(); it != bookPaths.end(); ++it) {
        QString namePath = (*it).path;
        QStringList dirs = namePath.split(QLatin1Char('/'));

        QStringList::iterator it1 = currentNamePath.begin();
        QStringList::iterator it2 = dirs.begin();
        for (; (it1 != currentNamePath.end())
             && (it2 != dirs.end()) && (*it1 == *it2); ++it1, ++it2);

        for (; it1 != currentNamePath.end(); ++it1) {
            parent = stack.pop();
        }
        for (; it2 != dirs.end(); ++it2) {
            stack.push(parent);
            QStandardItem *newParent = new QStandardItem(*it2);
            newParent->setCheckable(true);
            parent->appendRow(newParent);
            parent = newParent;
        }
        currentNamePath = dirs;

        QStandardItem *book;
        book = new QStandardItem((*it).name);
        book->setData((*it).filename);
        book->setCheckable(true);
        parent->appendRow(book);
    }
}

void InitialPhraseBookWidget::createBook()
{
    PhraseBook book;
    QStandardItem *item = m_model->invisibleRootItem();
    addChildrenToBook(book, item);

    QString bookLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1Char('/');
    if (!bookLocation.isNull() && !bookLocation.isEmpty()) {
        QDir().mkpath(bookLocation);
        qDebug() << "creating book at location " << bookLocation;
        book.save(QUrl::fromLocalFile(bookLocation + QStringLiteral("standard.phrasebook")));
    }
}

void InitialPhraseBookWidget::addChildrenToBook(PhraseBook &book, QStandardItem *item)
{
    for (int i = 0; i < item->rowCount(); ++i) {
        QStandardItem *child = item->child(i);
        if (child->checkState() != Qt::Unchecked) {
            PhraseBook localBook;
            if (localBook.open(QUrl::fromLocalFile(child->data().toString()))) {
                book += localBook;
            }
        }
        if (child->hasChildren())
            addChildrenToBook(book, child);
    }
}
