/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "number_format.h"
#include "ohos/init_data.h"
#include <locale>
#include <codecvt>

namespace OHOS {
namespace Global {
namespace I18n {
std::map<std::string, UNumberFormatStyle> NumberFormat::formatStyle = {
    {"decimal", UNumberFormatStyle::UNUM_DECIMAL },
    { "currency", UNumberFormatStyle::UNUM_CURRENCY },
    { "percent", UNumberFormatStyle::UNUM_PERCENT }
};

NumberFormat::NumberFormat(const std::vector<std::string> &localeTags, std::map<std::string, std::string> &configs)
{
    UErrorCode status = U_ZERO_ERROR;
    auto builder = std::make_unique<icu::LocaleBuilder>();
    ParseConfigs(configs);
    GetValidLocales();
    for (size_t i = 0; i < localeTags.size(); i++) {
        std::string curLocale = localeTags[i];
        locale = builder->setLanguageTag(icu::StringPiece(curLocale)).build(status);
        if (allValidLocales.count(locale.getLanguage()) > 0) {
            localeInfo = new LocaleInfo(curLocale, configs);
            locale = localeInfo->GetLocale();
            localeBaseName = localeInfo->GetBaseName();
            numberFormat = icu::NumberFormat::createInstance(locale, style, status);
            if (numberFormat == nullptr) {
                continue;
            }
            break;
        }
    }
    if (numberFormat == nullptr) {
        locale = icu::Locale::getDefault();
        localeBaseName = locale.getBaseName();
        std::replace(localeBaseName.begin(), localeBaseName.end(), '_', '-');
        numberFormat = icu::NumberFormat::createInstance(status);
    }
    InitProperties();
}

NumberFormat::~NumberFormat()
{
    if (numberFormat != nullptr) {
        delete numberFormat;
        numberFormat = nullptr;
    }
}

void NumberFormat::GetValidLocales()
{
    int32_t validCount = 1;
    const icu::Locale *validLocales = icu::Locale::getAvailableLocales(validCount);
    for (int i = 0; i < validCount; i++) {
        allValidLocales.insert(validLocales[i].getLanguage());
    }
}

void NumberFormat::InitProperties()
{
    if (!currency.empty()) {
        UErrorCode status = U_ZERO_ERROR;
        numberFormat->setCurrency(
             std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(currency).c_str(), status);
    }
    if (!useGrouping.empty()) {
        numberFormat->setGroupingUsed(useGrouping == "true");
    }
    if (!minimumIntegerDigits.empty()) {
        numberFormat->setMinimumIntegerDigits(std::stoi(minimumIntegerDigits));
    }
    if (!minimumFractionDigits.empty()) {
        numberFormat->setMinimumFractionDigits(std::stoi(minimumFractionDigits));
    }
    if (!maximumFractionDigits.empty()) {
        numberFormat->setMaximumFractionDigits(std::stoi(maximumFractionDigits));
    }
}

void NumberFormat::ParseConfigs(std::map<std::string, std::string> &configs)
{
    if (configs.count("currency") > 0) {
        currency = configs["currency"];
    }
    if (configs.count("currencySign") > 0) {
        currencySign = configs["currencySign"];
    }
    if (configs.count("style") > 0) {
        styleString = configs["style"];
        if (formatStyle.count(styleString)) {
            style = formatStyle[styleString];
        }
    }
    if (configs.count("numberingSystem")) {
        numberingSystem = configs["numberingSystem"];
    }
    if (configs.count("useGrouping")) {
        useGrouping = configs["useGrouping"];
    }
    if (configs.count("minimumIntegerDigits")) {
        minimumIntegerDigits = configs["minimumIntegerDigits"];
    }
    if (configs.count("minimumFractionDigits")) {
        minimumFractionDigits = configs["minimumFractionDigits"];
    }
    if (configs.count("maximumFractionDigits")) {
        maximumFractionDigits = configs["maximumFractionDigits"];
    }
}

bool NumberFormat::icuInitialized = NumberFormat::Init();

std::string NumberFormat::Format(double number)
{
    std::string result;
    icu::UnicodeString numberString;
    numberFormat->format(number, numberString);
    numberString.toUTF8String(result);
    return result;
}

void NumberFormat::GetResolvedOptions(std::map<std::string, std::string> &map)
{
    UErrorCode status = U_ZERO_ERROR;
    map.insert(std::make_pair("locale", localeBaseName));
    if (!styleString.empty()) {
        map.insert(std::make_pair("style", styleString));
    }
    if (!currency.empty()) {
        map.insert(std::make_pair("currency", currency));
    }
    if (!currencySign.empty()) {
        map.insert(std::make_pair("currencySign", currencySign));
    }
    if (!numberingSystem.empty()) {
        map.insert(std::make_pair("numberingSystem", numberingSystem));
    } else if (!(localeInfo->GetNumberingSystem()).empty()) {
        map.insert(std::make_pair("numberingSystem", localeInfo->GetNumberingSystem()));
    } else {
        icu::NumberingSystem *numSys = icu::NumberingSystem::createInstance(locale, status);
        map.insert(std::make_pair("numberingSystem", numSys->getName()));
        delete numSys;
    }
    if (!useGrouping.empty()) {
        map.insert(std::make_pair("useGrouping", useGrouping));
    }
    if (!minimumIntegerDigits.empty()) {
        map.insert(std::make_pair("minimumIntegerDigits", minimumIntegerDigits));
    }
    if (!minimumFractionDigits.empty()) {
        map.insert(std::make_pair("minimumFractionDigits", minimumFractionDigits));
    }
    if (!maximumFractionDigits.empty()) {
        map.insert(std::make_pair("maximumFractionDigits", maximumFractionDigits));
    }
}

std::string NumberFormat::GetCurrency()
{
    return currency;
}

std::string NumberFormat::GetCurrencySign()
{
    return currencySign;
}

std::string NumberFormat::GetStyle()
{
    return styleString;
}

std::string NumberFormat::GetNumberingSystem()
{
    return numberingSystem;
}

std::string NumberFormat::GetUseGrouping()
{
    return useGrouping;
}

std::string NumberFormat::GetMinimumIntegerDigits()
{
    return minimumIntegerDigits;
}

std::string NumberFormat::GetMinimumFractionDigits()
{
    return minimumFractionDigits;
}

std::string NumberFormat::GetMaximumFractionDigits()
{
    return maximumFractionDigits;
}

bool NumberFormat::Init()
{
    SetHwIcuDirectory();
    return true;
}

} // namespace I18n
} // namespace Global
} // namespace OHOS
