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
#include <vector>
#include "i18n_addon.h"
#include "hilog/log.h"
#include "node_api.h"

namespace OHOS {
namespace Global {
namespace I18n {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, 0xD001E00, "I18nJs" };
using namespace OHOS::HiviewDFX;

I18nAddon::I18nAddon() : env_(nullptr), wrapper_(nullptr) {}

I18nAddon::~I18nAddon()
{
    napi_delete_reference(env_, wrapper_);
}

void I18nAddon::Destructor(napi_env env, void *nativeObject, void *hint)
{
    if (nativeObject == nullptr) {
        return;
    }
    reinterpret_cast<I18nAddon *>(nativeObject)->~I18nAddon();
}

napi_value I18nAddon::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getSystemLanguages", GetSystemLanguages),
        DECLARE_NAPI_FUNCTION("getSystemCountries", GetSystemCountries),
        DECLARE_NAPI_FUNCTION("isSuggested", IsSuggested),
        DECLARE_NAPI_FUNCTION("getDisplayLanguage", GetDisplayLanguage),
        DECLARE_NAPI_FUNCTION("getDisplayCountry", GetDisplayCountry),
        DECLARE_NAPI_FUNCTION("getSystemLanguage", GetSystemLanguage),
        DECLARE_NAPI_FUNCTION("getSystemRegion", GetSystemRegion),
        DECLARE_NAPI_FUNCTION("getSystemLocale", GetSystemLocale),
        DECLARE_NAPI_FUNCTION("setSystemLanguage", SetSystemLanguage),
        DECLARE_NAPI_FUNCTION("setSystemRegion", SetSystemRegion),
        DECLARE_NAPI_FUNCTION("setSystemLocale", SetSystemLocale),
    };

    status = napi_define_properties(env, exports,
                                    sizeof(properties) / sizeof(napi_property_descriptor),
                                    properties);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to set properties at init");
        return nullptr;
    }
    return exports;
}

napi_value I18nAddon::GetSystemLanguages(napi_env env, napi_callback_info info)
{
    std::vector<std::string> systemLanguages;
    LocaleConfig::GetSystemLanguages(systemLanguages);
    napi_value result;
    napi_status status = napi_create_array_with_length(env, systemLanguages.size(), &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create array");
        return nullptr;
    }
    for (size_t i = 0; i < systemLanguages.size(); i++) {
        napi_value value;
        status = napi_create_string_utf8(env, systemLanguages[i].c_str(), NAPI_AUTO_LENGTH, &value);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "Failed to create string item");
            return nullptr;
        }
        status = napi_set_element(env, result, i, value);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "Failed to set array item");
            return nullptr;
        }
    }
    return result;
}

napi_value I18nAddon::GetSystemCountries(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    size_t len;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    std::vector<char> localeBuf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], localeBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get string item");
        return nullptr;
    }
    std::vector<std::string> systemCountries;
    LocaleConfig::GetSystemCountries(systemCountries);
    napi_value result;
    status = napi_create_array_with_length(env, systemCountries.size(), &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create array");
        return nullptr;
    }
    for (size_t i = 0; i < systemCountries.size(); i++) {
        napi_value value;
        status = napi_create_string_utf8(env, systemCountries[i].c_str(), NAPI_AUTO_LENGTH, &value);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "Failed to create string item");
            return nullptr;
        }
        status = napi_set_element(env, result, i, value);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "Failed to set array item");
            return nullptr;
        }
    }
    return result;
}

napi_value I18nAddon::GetSystemLanguage(napi_env env, napi_callback_info info)
{
    std::string value = LocaleConfig::GetSystemLanguage();
    napi_value result;
    napi_status status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create string item");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::GetSystemRegion(napi_env env, napi_callback_info info)
{
    std::string value = LocaleConfig::GetSystemRegion();
    napi_value result;
    napi_status status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create string item");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::GetSystemLocale(napi_env env, napi_callback_info info)
{
    std::string value = LocaleConfig::GetSystemLocale();
    napi_value result;
    napi_status status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create string item");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::GetDisplayLanguage(napi_env env, napi_callback_info info)
{
    // Need to get three parameters to get the display Language.
    size_t argc = 3;
    napi_value argv[3] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    size_t len;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    std::vector<char> localeBuf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], localeBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get string item");
        return nullptr;
    }
    napi_get_value_string_utf8(env, argv[1], nullptr, 0, &len);
    std::vector<char> displayLocaleBuf(len + 1);
    status = napi_get_value_string_utf8(env, argv[1], displayLocaleBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get string item");
        return nullptr;
    }
    bool sentenceCase = true;
    int sentenceCaseIndex = 2;
    if (argv[sentenceCaseIndex] != nullptr) {
        napi_get_value_bool(env, argv[sentenceCaseIndex], &sentenceCase);
    }

    std::string value = LocaleConfig::GetDisplayLanguage(localeBuf.data(), displayLocaleBuf.data(), sentenceCase);
    napi_value result;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create string item");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::GetDisplayCountry(napi_env env, napi_callback_info info)
{
    // Need to get three parameters to get the display country.
    size_t argc = 3;
    napi_value argv[3] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    size_t len;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    std::vector<char> localeBuf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], localeBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get string item");
        return nullptr;
    }
    napi_get_value_string_utf8(env, argv[1], nullptr, 0, &len);
    std::vector<char> displayLocaleBuf(len + 1);
    status = napi_get_value_string_utf8(env, argv[1], displayLocaleBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get string item");
        return nullptr;
    }
    bool sentenceCase = true;
    int sentenceCaseIndex = 2;
    if (argv[sentenceCaseIndex] != nullptr) {
        napi_get_value_bool(env, argv[sentenceCaseIndex], &sentenceCase);
    }
    std::string value = LocaleConfig::GetDisplayRegion(localeBuf.data(), displayLocaleBuf.data(), sentenceCase);
    napi_value result;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create string item");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::IsSuggested(napi_env env, napi_callback_info info)
{
    // Need to get two parameters to check is suggested or not.
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    size_t len;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    std::vector<char> languageBuf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], languageBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get string item");
        return nullptr;
    }
    bool isSuggested = false;
    if (argv[1] != nullptr) {
        napi_get_value_string_utf8(env, argv[1], nullptr, 0, &len);
        std::vector<char> regionBuf(len + 1);
        status = napi_get_value_string_utf8(env, argv[1], regionBuf.data(), len + 1, &len);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "Failed to get string item");
            return nullptr;
        }
        isSuggested = LocaleConfig::IsSuggested(languageBuf.data(), regionBuf.data());
    } else {
        isSuggested = LocaleConfig::IsSuggested(languageBuf.data());
    }
    napi_value result;
    status = napi_get_boolean(env, isSuggested, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create case first boolean value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::SetSystemLanguage(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    size_t len;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    std::vector<char> languageBuf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], languageBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get string item");
        return nullptr;
    }
    bool success = LocaleConfig::SetSystemLanguage(languageBuf.data());
    napi_value result;
    status = napi_get_boolean(env, success, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create set system language boolean value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::SetSystemRegion(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    size_t len;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    std::vector<char> regionBuf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], regionBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get string item");
        return nullptr;
    }
    bool success = LocaleConfig::SetSystemRegion(regionBuf.data());
    napi_value result;
    status = napi_get_boolean(env, success, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create set system language boolean value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::SetSystemLocale(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    size_t len;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    std::vector<char> localeBuf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], localeBuf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get string item");
        return nullptr;
    }
    bool success = LocaleConfig::SetSystemLocale(localeBuf.data());
    napi_value result;
    status = napi_get_boolean(env, success, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create set system language boolean value failed");
        return nullptr;
    }
    return result;
}

napi_value I18nAddon::InitPhoneNumberFormat(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("isValidNumber", IsValidPhoneNumber),
        DECLARE_NAPI_FUNCTION("format", FormatPhoneNumber)
    };

    napi_value constructor;
    status = napi_define_class(env, "PhoneNumberFormat", NAPI_AUTO_LENGTH, PhoneNumberFormatConstructor, nullptr,
                               sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Define class failed when InitPhoneNumberFormat");
        return nullptr;
    }

    status = napi_set_named_property(env, exports, "PhoneNumberFormat", constructor);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Set property failed when InitPhoneNumberFormat");
        return nullptr;
    }
    return exports;
}

void GetOptionValue(napi_env env, napi_value options, const std::string &optionName,
                    std::map<std::string, std::string> &map)
{
    napi_value optionValue = nullptr;
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, options, &type);
    if (status != napi_ok && type != napi_object) {
        HiLog::Error(LABEL, "Get option failed, option is not an object");
        return;
    }
    bool hasProperty = false;
    napi_status propStatus = napi_has_named_property(env, options, optionName.c_str(), &hasProperty);
    if (propStatus == napi_ok && hasProperty) {
        status = napi_get_named_property(env, options, optionName.c_str(), &optionValue);
        if (status == napi_ok) {
            size_t len;
            napi_get_value_string_utf8(env, optionValue, nullptr, 0, &len);
            std::vector<char> optionBuf(len + 1);
            status = napi_get_value_string_utf8(env, optionValue, optionBuf.data(), len + 1, &len);
            map.insert(make_pair(optionName, optionBuf.data()));
        }
    }
}

napi_value I18nAddon::PhoneNumberFormatConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }

    size_t len;
    status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get country tag length failed");
        return nullptr;
    }

    std::vector<char> country (len + 1);
    status = napi_get_value_string_utf8(env, argv[0], country.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get country tag failed");
        return nullptr;
    }

    std::map<std::string, std::string> options;
    GetOptionValue(env, argv[1], "type", options);

    std::unique_ptr<I18nAddon> obj = std::make_unique<I18nAddon>();
    if (obj == nullptr) {
        HiLog::Error(LABEL, "Create IntlAddon failed");
        return nullptr;
    }

    status = napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()),
                       I18nAddon::Destructor, nullptr, &obj->wrapper_);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Wrap IntlAddon failed");
        return nullptr;
    }

    if (!obj->InitPhoneNumberFormatContext(env, info, country.data(), options)) {
        return nullptr;
    }

    obj.release();

    return thisVar;
}

bool I18nAddon::InitPhoneNumberFormatContext(napi_env env, napi_callback_info info, const std::string &country,
                                             const std::map<std::string, std::string> &options)
{
    napi_value global;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get global failed");
        return false;
    }
    env_ = env;
    phonenumberfmt_ = std::make_unique<PhoneNumberFormat>(country, options);

    return phonenumberfmt_ != nullptr;
}

napi_value I18nAddon::IsValidPhoneNumber(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }

    size_t len;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get phone number length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get phone number failed");
        return nullptr;
    }

    I18nAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->phonenumberfmt_ == nullptr) {
        HiLog::Error(LABEL, "GetPhoneNumberFormat object failed");
        return nullptr;
    }

    bool isValid = obj->phonenumberfmt_->isValidPhoneNumber(buf.data());

    napi_value result;
    status = napi_get_boolean(env, isValid, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create boolean failed");
        return nullptr;
    }

    return result;
}

napi_value I18nAddon::FormatPhoneNumber(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }

    size_t len;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get phone number length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get phone number failed");
        return nullptr;
    }

    I18nAddon *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->phonenumberfmt_ == nullptr) {
        HiLog::Error(LABEL, "Get PhoneNumberFormat object failed");
        return nullptr;
    }

    std::string formattedPhoneNumber = obj->phonenumberfmt_->format(buf.data());

    napi_value result;
    status = napi_create_string_utf8(env, formattedPhoneNumber.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create format phone number failed");
        return nullptr;
    }
    return result;
}


napi_value Init(napi_env env, napi_value exports)
{
    napi_value val = I18nAddon::Init(env, exports);
    return I18nAddon::InitPhoneNumberFormat(env, val);
}

static napi_module g_i18nModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "i18n",
    .nm_priv = ((void *)0),
    .reserved = { 0 }
};

extern "C" __attribute__((constructor)) void I18nRegister()
{
    napi_module_register(&g_i18nModule);
}
} // namespace I18n
} // namespace Global
} // namespace OHOS