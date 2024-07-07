// Copyright (c) 2011-2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

#include <qt/splashscreen.h>

#include <clientversion.h>
#include <interfaces/handler.h>
#include <interfaces/node.h>
#include <interfaces/wallet.h>
#include <qt/guiutil.h>
#include <qt/networkstyle.h>
#include <qt/walletmodel.h>
#include <util/system.h>
#include <util/translation.h>

#include <functional>

#include <QApplication>
#include <QCloseEvent>
#include <QPainter>
#include <QRadialGradient>
#include <QScreen>


SplashScreen::SplashScreen(const NetworkStyle* networkStyle)
    : QWidget(nullptr, Qt::FramelessWindowHint), curAlignment(0)
{
    // set reference point, paddings
    int paddingRight            = 30;
    int paddingTop              = 50;
    int titleVersionVSpace      = 17;
    int titleCopyrightVSpace    = 40;
    int titleCoinVSpace         = 25;

    float fontFactor            = 1.0;
    float devicePixelRatio      = 1.0;
    devicePixelRatio = static_cast<QGuiApplication*>(QCoreApplication::instance())->devicePixelRatio();

    // define text to place
    QString titleText       = PACKAGE_NAME;
    QString coinText       = "MateableCoin";

    QString versionText     = QString("Version %1").arg(QString::fromStdString(FormatFullVersion()));
    //QString copyrightText   = QString::fromUtf8(CopyrightHolders(strprintf("\xc2\xA9 %u-%u ", 2009, COPYRIGHT_YEAR)).c_str());
    const QString& titleAddText    = networkStyle->getTitleAddText();

    QString font            = QApplication::font().toString();

    // create a bitmap according to device pixelratio
    QSize splashSize(480*devicePixelRatio,320*devicePixelRatio);
    pixmap = QPixmap(splashSize);

    // change to HiDPI if it makes sense
    pixmap.setDevicePixelRatio(devicePixelRatio);

    QPainter pixPaint(&pixmap);
    pixPaint.fillRect(pixmap.rect(), Qt::black); // Clearing pixmap with black color
    pixPaint.setPen(Qt::white); // Set text color to white
    QPixmap backgroundImage(":/icons/splash");
    backgroundImage = backgroundImage.scaled(pixmap.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixPaint.drawPixmap(0, 0, backgroundImage);

    // draw a slightly radial gradient
    //QRadialGradient gradient(QPoint(0,0), splashSize.width()/devicePixelRatio);
    //gradient.setColorAt(0, Qt::white);
    //gradient.setColorAt(1, QColor(247,247,247));
    //QRect rGradient(QPoint(0,0), splashSize);
    //pixPaint.fillRect(rGradient, gradient);

// Calculate the desired size for the icon
const int iconSize = 180; // Adjust this value as needed

// Calculate the position for the icon based on the splash screen size
int iconX = (splashSize.width() - iconSize) / 2 - 380; // Adjust this value to move the icon towards the rightint iconY = paddingTop; // Adjust this value to position the icon closer to the top// Define the QRect for the icon
int iconY = paddingTop - 40; // Adjust this value to position the icon closer to the top

QRect rectIcon(QPoint(iconX, iconY), QSize(iconSize, iconSize));

// Load the icon with the desired size
QPixmap icon(networkStyle->getAppIcon().pixmap(iconSize, iconSize));
    pixPaint.drawPixmap(rectIcon, icon);
// Calculate the position for the coin text
int coinTextX = (splashSize.width() - QFontMetrics(QFont(font, 15 * fontFactor)).width(coinText)) / 2 - 380;
int coinTextY = iconY + iconSize + titleCoinVSpace; // Adjust this value to position the coin text below the icon

// Define colors for the text
QColor mateableColor = QColor(Qt::white);
QColor coinColor = QColor(Qt::white);

// Draw the "Mateable" text
pixPaint.setFont(QFont(font, 15 * fontFactor, QFont::Bold));
pixPaint.setPen(mateableColor); // Set the color for the "Mateable" text
pixPaint.drawText(coinTextX, coinTextY, "Mateable");

// Calculate the position for the "Coin" text within the same line
int coinTextWidth = QFontMetrics(QFont(font, 15 * fontFactor)).width("Mateable");
coinTextX += coinTextWidth; // Adjust the position to follow "Mateable"

// Add an offset to ensure the "Coin" text starts after "Mateable" without overlapping
int coinTextXOffset = 13; // Adjust this value as needed to add spacing between "Mateable" and "Coin"
coinTextX += coinTextXOffset;

// Draw the "Coin" text
pixPaint.setPen(coinColor); // Set the color for the "Coin" text
pixPaint.drawText(coinTextX, coinTextY, "Coin");

    // check font size and drawing with
    pixPaint.setFont(QFont(font, 33*fontFactor, QFont::Bold));
    QFontMetrics fm = pixPaint.fontMetrics();
    int titleTextWidth = GUIUtil::TextWidth(fm, titleText);
    if (titleTextWidth > 176) {
        fontFactor = fontFactor * 176 / titleTextWidth;
    }

    // Calculate the position for the title text
    int titleX = pixmap.width() / devicePixelRatio - titleTextWidth - paddingRight;

    pixPaint.setFont(QFont(font, 33*fontFactor, QFont::Bold));
    fm = pixPaint.fontMetrics();
    titleTextWidth  = GUIUtil::TextWidth(fm, titleText);
    pixPaint.drawText(pixmap.width()/devicePixelRatio-titleTextWidth-paddingRight,paddingTop,titleText);

    pixPaint.setFont(QFont(font, 15*fontFactor, QFont::Bold));

    // if the version string is too long, reduce size
    fm = pixPaint.fontMetrics();
    int versionTextWidth  = GUIUtil::TextWidth(fm, versionText);
    if(versionTextWidth > titleTextWidth+paddingRight-10) {
        pixPaint.setFont(QFont(font, 10*fontFactor));
        titleVersionVSpace -= 5;
    }
    pixPaint.drawText(pixmap.width()/devicePixelRatio-titleTextWidth-paddingRight+2,paddingTop+titleVersionVSpace,versionText);
    

    // Calculate the position for the copyright text
    int copyrightX = titleX + 90; // Adjust this value to position the copyright text slightly to the left of the title text

// Draw copyright stuff
{
    pixPaint.setFont(QFont(font, 15*fontFactor, QFont::Bold));
    const int y = paddingTop + titleCopyrightVSpace;
    QRect copyrightRect(copyrightX, y, pixmap.width() - copyrightX - paddingRight, pixmap.height() - y);
    pixPaint.drawText(copyrightRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, getClientCopyright());
}
    // draw additional text if special network
   if(!titleAddText.isEmpty()) {
        QFont boldFont = QFont(font, 10*fontFactor);
        boldFont.setWeight(QFont::Bold);
        pixPaint.setFont(boldFont);
        fm = pixPaint.fontMetrics();
        int titleAddTextWidth  = GUIUtil::TextWidth(fm, titleAddText);
        pixPaint.drawText(pixmap.width()/devicePixelRatio-titleAddTextWidth-10,15,titleAddText);
    }

    pixPaint.end();

    // Set window title
    setWindowTitle(titleText + " " + titleAddText);

    // Resize window and move to center of desktop, disallow resizing
    QRect r(QPoint(), QSize(pixmap.size().width()/devicePixelRatio,pixmap.size().height()/devicePixelRatio));
    resize(r.size());
    setFixedSize(r.size());
    move(QGuiApplication::primaryScreen()->geometry().center() - r.center());

    installEventFilter(this);

    GUIUtil::handleCloseWindowShortcut(this);
}

QString SplashScreen::getClientCopyright()
{
    std::string clientName(CLIENT_NAME);

    // Convert clientName to lowercase for case-insensitive comparison
    std::transform(clientName.begin(), clientName.end(), clientName.begin(), ::tolower);

    size_t mateableIndex = clientName.find("mateable");
    size_t bitcoinIndex = clientName.find("bitcoin");

    QString mateableCopyright = QString("\u00A9 %1-%2 The Mateable Core Developers")
        .arg(QString::number(2022), QString::number(COPYRIGHT_YEAR));

    QString bitcoinCopyright = QString("\u00A9 %1-%2 The Bitcoin Core Developers")
        .arg(QString::number(2009), QString::number(COPYRIGHT_YEAR));

    if (mateableIndex != std::string::npos && bitcoinIndex != std::string::npos) {
        // Both Mateable and Bitcoin Core found
        return mateableCopyright + "\n" + bitcoinCopyright;
    }

    // Neither Mateable nor Bitcoin Core found
    return "";
}

SplashScreen::~SplashScreen()
{
    if (m_node) unsubscribeFromCoreSignals();
}

void SplashScreen::setNode(interfaces::Node& node)
{
    assert(!m_node);
    m_node = &node;
    subscribeToCoreSignals();
    if (m_shutdown) m_node->startShutdown();
}

void SplashScreen::shutdown()
{
    m_shutdown = true;
    if (m_node) m_node->startShutdown();
}

bool SplashScreen::eventFilter(QObject * obj, QEvent * ev) {
    if (ev->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(ev);
        if (keyEvent->key() == Qt::Key_Q) {
            shutdown();
        }
    }
    return QObject::eventFilter(obj, ev);
}

void SplashScreen::finish()
{
    /* If the window is minimized, hide() will be ignored. */
    /* Make sure we de-minimize the splashscreen window before hiding */
    if (isMinimized())
        showNormal();
    hide();
    deleteLater(); // No more need for this
}
static void InitMessage(SplashScreen *splash, const std::string &message)
{
    bool invoked = QMetaObject::invokeMethod(splash, "showMessage",
        Qt::QueuedConnection,
        Q_ARG(QString, QString::fromStdString(message)),
        Q_ARG(int, Qt::AlignBottom|Qt::AlignHCenter),
        Q_ARG(QColor, QColor(255, 255, 255))); // Change color to white
    assert(invoked);
}

static void ShowProgress(SplashScreen *splash, const std::string &title, int nProgress, bool resume_possible)
{
    InitMessage(splash, title + std::string("\n") +
            (resume_possible ? SplashScreen::tr("(press q to shutdown and continue later)").toStdString()
                                : SplashScreen::tr("press q to shutdown").toStdString()) +
            strprintf("\n%d", nProgress) + "%");
}

void SplashScreen::subscribeToCoreSignals()
{
    // Connect signals to client
    m_handler_init_message = m_node->handleInitMessage(std::bind(InitMessage, this, std::placeholders::_1));
    m_handler_show_progress = m_node->handleShowProgress(std::bind(ShowProgress, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_handler_init_wallet = m_node->handleInitWallet([this]() { handleLoadWallet(); });
}

void SplashScreen::handleLoadWallet()
{
#ifdef ENABLE_WALLET
    if (!WalletModel::isWalletEnabled()) return;
    m_handler_load_wallet = m_node->walletLoader().handleLoadWallet([this](std::unique_ptr<interfaces::Wallet> wallet) {
        m_connected_wallet_handlers.emplace_back(wallet->handleShowProgress(std::bind(ShowProgress, this, std::placeholders::_1, std::placeholders::_2, false)));
        m_connected_wallets.emplace_back(std::move(wallet));
    });
#endif
}

void SplashScreen::unsubscribeFromCoreSignals()
{
    // Disconnect signals from client
    m_handler_init_message->disconnect();
    m_handler_show_progress->disconnect();
    for (const auto& handler : m_connected_wallet_handlers) {
        handler->disconnect();
    }
    m_connected_wallet_handlers.clear();
    m_connected_wallets.clear();
}

void SplashScreen::showMessage(const QString &message, int alignment, const QColor &color)
{
    curMessage = message;
    curAlignment = alignment;
    curColor = color;
    update();
}

void SplashScreen::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, pixmap);
    QRect r = rect().adjusted(5, 5, -5, -5);
    painter.setPen(curColor);

    painter.drawText(r, curAlignment, curMessage);
}

void SplashScreen::closeEvent(QCloseEvent *event)
{
    shutdown(); // allows an "emergency" shutdown during startup
    event->ignore();
}
