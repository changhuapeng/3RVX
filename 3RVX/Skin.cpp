#include "Skin.h"

#include <algorithm>
#include "MeterWnd/Meters/MeterTypes.h"

Gdiplus::Bitmap *Skin::OSDBgImg(char *osdName) {
    tinyxml2::XMLElement *osd = OSDXMLElement(osdName);
    const char *bgFile = osd->Attribute("background");
    if (bgFile == NULL) {
        CLOG(L"Could not load OSD background image.");
        return NULL;
    }
    std::wstring wBgFile = _skinDir + L"\\" + Widen(bgFile);
    Gdiplus::Bitmap *bg = Gdiplus::Bitmap::FromFile(wBgFile.c_str());
    return bg;
}

void Skin::Meters(char *osdName) {
    tinyxml2::XMLElement *osd = OSDXMLElement(osdName);
    tinyxml2::XMLElement *meter = osd->FirstChildElement("meter");
    LoadMeter(meter);
    const char *img = meter->Attribute("image");
    printf("%s\n", img);
}

Meter *Skin::LoadMeter(tinyxml2::XMLElement *meterXMLElement) {
    const char *meterType = meterXMLElement->Attribute("type");
    if (meterType == NULL) {
        /* If we dont' know the meter type, we can't proceed. */
        CLOG(L"Unknown meter type!");
        return NULL;
    }

    std::string type(meterType);
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);

    /* IntAttribute() returns 0 on error, the default position. */
    int x = meterXMLElement->IntAttribute("x");
    int y = meterXMLElement->IntAttribute("y");

    int units = 10;
    meterXMLElement->QueryIntAttribute("units", &units);

    /* Check for meter background image. 'text' is the only meter
     * that does not require an image. */
    std::wstring img;
    if (type != "text") {
        img = ImageName(meterXMLElement);
        if (FileExists(img) == false) {
            CLOG(L"Could not find meter bitmap: %s", img.c_str());
            return NULL;
        }
    }

    Meter *m = NULL;
    if (type == "bitstrip") {
        m = new Bitstrip(img, x, y, units);
    } else if (type == "horizontalendcap") {
        m = new HorizontalEndcap(img, x, y, units);
    } else if (type == "horizontalbar") {
        m = new HorizontalEndcap(img, x, y, units);
    } else if (type == "horizontaltile") {
        m = new HorizontalTile(img, x, y, units);
    } else {
        CLOG(L"Unknown meter type: %s", Widen(type).c_str());
        return NULL;
    }

    CLOG(L"Created meter [%s]:\n%s", Widen(type).c_str(), m->ToString().c_str());

    return m;
}

std::wstring Skin::ImageName(tinyxml2::XMLElement *meterXMLElement) {
    const char *imgName = meterXMLElement->Attribute("image");
    if (imgName == NULL) {
        return NULL;
    }
    return _skinDir + L"\\" + Widen(imgName);
}

tinyxml2::XMLElement *Skin::OSDXMLElement(char *osdName) {
    tinyxml2::XMLElement *osd = _xml.GetDocument()
        ->FirstChildElement("skin")
        ->FirstChildElement("osds")
        ->FirstChildElement(osdName);
    return osd;
}

bool Skin::FileExists(std::wstring fileName) {
    DWORD attr = GetFileAttributes(fileName.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES
        && !(attr & FILE_ATTRIBUTE_DIRECTORY)
        && GetLastError() != ERROR_FILE_NOT_FOUND);
}

std::wstring Skin::Widen(const char *str) {
    std::string sstr(str);
    return Widen(sstr);
}

std::wstring Skin::Widen(std::string &str) {
    int size = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int) str.size(),
        NULL, 0);
    std::wstring buf(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int) str.size(), &buf[0], size);
    return buf;
}

std::string Skin::Narrow(std::wstring &str) {
    int size = WideCharToMultiByte(CP_UTF8, 0, &str[0], (int) str.size(),
        NULL, 0, NULL, NULL);
    std::string buf(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, &str[0], (int) str.size(),
        &buf[0], size, NULL, NULL);
    return buf;
}