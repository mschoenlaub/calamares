/* === This file is part of Calamares - <https://github.com/calamares> ===
 *
 *   Copyright 2020, Adriaan de Groot <groot@kde.org>
 *
 *   Calamares is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Calamares is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Calamares. If not, see <http://www.gnu.org/licenses/>.
 */

#include "QmlViewStep.h"

#include "utils/Dirs.h"
#include "utils/Logger.h"
#include "widgets/WaitingWidget.h"

#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWidget>
#include <QVBoxLayout>
#include <QWidget>

namespace Calamares
{

QmlViewStep::QmlViewStep( const QString& name, QObject* parent )
    : ViewStep( parent )
    , m_name( name )
    , m_widget( new QWidget )
    , m_spinner( new WaitingWidget( tr( "Loading ..." ) ) )
    , m_qmlWidget( new QQuickWidget )
{
    QVBoxLayout* layout = new QVBoxLayout( m_widget );
    layout->addWidget( m_spinner );

    m_qmlWidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    m_qmlWidget->setResizeMode( QQuickWidget::SizeRootObjectToView );
    m_qmlWidget->engine()->addImportPath( CalamaresUtils::qmlModulesDir().absolutePath() );

    // TODO: search for suitable file
    QString qrcName = QStringLiteral( ":/%1.qml" ).arg( m_name );
    m_qmlFileName = qrcName;

    cDebug() << "QmlViewStep loading" << m_qmlFileName;
    m_qmlComponent = new QQmlComponent(
        m_qmlWidget->engine(), QUrl( m_qmlFileName ), QQmlComponent::CompilationMode::Asynchronous );
    connect( m_qmlComponent, &QQmlComponent::statusChanged, this, &QmlViewStep::loadComplete );
    cDebug() << Logger::SubEntry << "Status" << m_qmlComponent->status();
}

QmlViewStep::~QmlViewStep() {}

QString
QmlViewStep::prettyName() const
{
    // TODO: query the QML itself
    return tr( "QML Step <i>%1</i>" ).arg( m_name );
}


}  // namespace Calamares

bool
Calamares::QmlViewStep::isAtBeginning() const
{
    return true;
}

bool
Calamares::QmlViewStep::isAtEnd() const
{
    return true;
}
bool
Calamares::QmlViewStep::isBackEnabled() const
{
    return true;
}

bool
Calamares::QmlViewStep::isNextEnabled() const
{
    return true;
}

Calamares::JobList
Calamares::QmlViewStep::jobs() const
{
    return JobList();
}

void
Calamares::QmlViewStep::onActivate()
{
    // TODO: call into QML
}

void
Calamares::QmlViewStep::onLeave()
{
    // TODO: call into QML
}

QWidget*
Calamares::QmlViewStep::widget()
{
    return m_widget;
}

void
Calamares::QmlViewStep::loadComplete()
{
    cDebug() << "QML component" << m_qmlFileName << m_qmlComponent->status();
    if ( m_qmlComponent->isReady() && !m_qmlObject )
    {
        cDebug() << "QML component complete" << m_qmlFileName;
        // Don't do this again
        disconnect( m_qmlComponent, &QQmlComponent::statusChanged, this, &QmlViewStep::loadComplete );

        QObject* o = m_qmlComponent->create();
        m_qmlObject = qobject_cast< QQuickItem* >( o );
        if ( !m_qmlObject )
        {
            cError() << Logger::SubEntry << "Could not create QML from" << m_qmlFileName;
            delete o;
        }
        else
        {
            // setContent() is public API, but not documented publicly.
            // It is marked \internal in the Qt sources, but does exactly
            // what is needed: sets up visual parent by replacing the root
            // item, and handling resizes.
            m_qmlWidget->setContent( QUrl( m_qmlFileName ), m_qmlComponent, m_qmlObject );
            showQml();
        }
    }
}

void
Calamares::QmlViewStep::showQml()
{
    if ( !m_qmlWidget || !m_qmlObject )
    {
        cDebug() << "showQml() called but no QML object";
        return;
    }
    if ( m_spinner )
    {
        m_widget->layout()->removeWidget( m_spinner );
        m_widget->layout()->addWidget( m_qmlWidget );
        delete m_spinner;
        m_spinner = nullptr;
    }
    else
    {
        cDebug() << "showQml() called twice";
    }
}