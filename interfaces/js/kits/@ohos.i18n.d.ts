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

 declare namespace i18n {
 export function getDisplayCountry(locale: string, displayLocale: string): Promise<string>;
 export function getDisplayCountry(locale: string, displayLocale: string, callback: AsyncCallback<string>);
 export function getDisplayLanguage(locale: string, displayLocale: string, sentenceCase: boolean): Promise<string>;
 export function getDisplayLanguage(locale: string, displayLocale: string, sentenceCase: boolean,
    callback: AsyncCallback<string>);
 export function getSystemLanguages(): Promise<Array<string>>;
 export function getSystemLanguages(callback: AsyncCallback<Array<string>>);
 export function getSystemCountries(language: string): Promise<Array<string>>;
 export function getSystemCountries(language: string, callback: AsyncCallback<Array<string>>);
 export function isSuggested(language: string, region: string): Promise<boolean>;
 export function isSuggested(language: string, region: string, callback: AsyncCallback<boolean>);
 export function getSystemLanguage(): Promise<string>;
 export function getSystemLanguage(callback: AsyncCallback<string>);
 export function setSystemLanguage(language: string);
 export function getSystemRegion(): Promise<string>;
 export function getSystemRegion(callback: AsyncCallback<string>);
 export function setSystemRegion(language: string);
 export function getSystemLocale(): Promise<string>;
 export function getSystemLocale(callback: AsyncCallback<string>);
 export function setSystemLocale(language: string);
 }