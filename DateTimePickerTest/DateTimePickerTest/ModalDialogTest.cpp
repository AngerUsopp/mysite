// ModalDialogTest.cpp : 实现文件
//

#include "stdafx.h"
#include "DateTimePickerTest.h"
#include "ModalDialogTest.h"
#include "afxdialogex.h"
#include "DateTimePickerTestDlg.h"
#include <string>
#include <vector>
#include <tuple>
#include <algorithm>

#include "curl/curl.h"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include <iostream>

// CModalDialogTest 对话框

IMPLEMENT_DYNAMIC(CModalDialogTest, CDialogEx)

CModalDialogTest::CModalDialogTest(CWnd* pParent /*=NULL*/)
	: CDialogEx(CModalDialogTest::IDD, pParent)
{
    id_ = inc_id_++;
}

CModalDialogTest::~CModalDialogTest()
{
}

void CModalDialogTest::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CModalDialogTest, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON1, &CModalDialogTest::OnBnClickedButton1)
END_MESSAGE_MAP()


int OnCurlDataRetrivedCallback(void* data,
    int size,
    int cnt,
    void* userData)
{
    int len = size * cnt;
    std::string *buff = static_cast<std::string*>(userData);
    if (buff)
    {
        buff->append((const char*)data, len);
    }

    return len;
}

void url_get()
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl)
    {
        std::string recv_data;
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080/test.php?p0=参数一&p1=参数二");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &OnCurlDataRetrivedCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&recv_data);
        res = curl_easy_perform(curl);

        if (CURLE_OK == res)
        {
            char *ct;
            /* ask for the content-type */
            res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);

            if ((CURLE_OK == res) && ct)
                printf("We received Content-Type: %s\n", ct);
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
}

int OnCUrlDataReadRequested(void* data,
    int size,
    int cnt,
    void* userData)
{
    std::tuple<std::string, int> *send_data = static_cast<std::tuple<std::string, int>*>(userData);
    int r_s = size * cnt;
    int ret = __min(r_s, std::get<0>(*send_data).length() - std::get<1>(*send_data));

    std::_Copy_n(std::get<0>(*send_data).data() + std::get<1>(*send_data), ret, static_cast<char*>(data));
    std::get<1>(*send_data) += ret;

    return ret;
}

void url_post(const std::string &json)
{
    CURL *curl;
    CURLcode res;

    /* In windows, this will init the winsock stuff */
    //curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();
    if (curl)
    {
        //curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080/test.php?p0=参数一&p1=参数二");
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080/test.php?p0=pa1&p1=pa2");
        std::string recv_data;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &OnCurlDataRetrivedCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&recv_data);
        
        curl_slist* headerList = nullptr;

        // WRITE JSON
        //std::tuple<std::string, int> send_data;
        ////std::get<0>(send_data) = "{\"name\":\"\\u6c49\\u5b57\",\"nick\":\"\\u6c49\\u5b57\",\"contact\":{\"email\":\"zhuoweida@163.com\",\"website\":\"http://zhuoweida.blog.tianya.cn\"}}";
        //std::get<0>(send_data) = "{\"name\":\"english\",\"nick\":\"english\"}";
        //std::get<1>(send_data) = 0;
        //std::string headerLength = std::string("Content-Length: ") + std::to_string(std::get<0>(send_data).length());
        //std::string headerType = std::string("Content-Type: application/json; charset=UTF-8");
        //curl_slist* headerList = curl_slist_append(NULL, headerLength.c_str());
        //headerList = curl_slist_append(headerList, "\r\n");
        //headerList = curl_slist_append(headerList, headerType.c_str());
        //curl_easy_setopt(curl, CURLOPT_POST, (long)1);
        //curl_easy_setopt(curl, CURLOPT_READFUNCTION, &OnCUrlDataReadRequested);
        //curl_easy_setopt(curl, CURLOPT_READDATA, (void*)&send_data);

        // WRITE JSON
        std::string headerType = std::string("Content-Type: application/json; charset=UTF-8");
        headerList = curl_slist_append(headerList, headerType.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

        if (headerList)
        {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, (curl_slist*)headerList);
        }

        res = curl_easy_perform(curl);
        if (headerList)
        {
            curl_slist_free_all(headerList);
        }

        if (CURLE_OK == res)
        {
            char *ct;
            /* ask for the content-type */
            res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);

            if ((CURLE_OK == res) && ct)
                printf("We received Content-Type: %s\n", ct);
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    //curl_global_cleanup();
}

#ifdef DEF_ON
void json_read()
{
    // ---------------------------- read json --------------------
    // parse json from string.
    rapidjson::Document;
    using rapidjson::Document;
    Document doc;
    doc.Parse<0>(stringFromStream.c_str());
    if (doc.HasParseError()) {
        rapidjson::ParseErrorCode code = doc.GetParseError();
        psln(code);
        return;
    }

    // use values in parse result.
    using rapidjson::Value;
    Value & v = doc["dictVersion"];
    if (v.IsInt()) {
        psln(v.GetInt());
    }

    Value & contents = doc["content"];
    if (contents.IsArray()) {
        for (size_t i = 0; i < contents.Size(); ++i) {
            Value & v = contents[i];
            assert(v.IsObject());
            if (v.HasMember("key") && v["key"].IsString()) {
                psln(v["key"].GetString());
            }
            if (v.HasMember("value") && v["value"].IsString()) {
                psln(v["value"].GetString());
            }
        }
    }
    // ---------------------------- write json --------------------
    pcln("add a value into array");

    Value item(Type::kObjectType);
    item.AddMember("key", "word5", doc.GetAllocator());
    item.AddMember("value", "单词5", doc.GetAllocator());
    contents.PushBack(item, doc.GetAllocator());

    // convert dom to string.
    StringBuffer buffer;      // in rapidjson/stringbuffer.h
    Writer<StringBuffer> writer(buffer); // in rapidjson/writer.h
    doc.Accept(writer);

    psln(buffer.GetString());
}
#endif

using namespace rapidjson;
using namespace std;

int json_parse()
{
    const char json[] = " { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ";

    // Parse a JSON string into DOM.
    //const char* json = "{\"project\":\"rapidjson\",\"stars\":10}";
    Document doc;
    doc.Parse(json);
    if (!doc.HasParseError())
    {
        if (!doc.ObjectEmpty())
        {
            const char *name = nullptr;
            Type tp = doc.GetType();
            Document::Object obj = doc.GetObject();
            for (Document::Object::MemberIterator iter = obj.begin(); iter != obj.end(); iter++)
            {
                name = iter->name.GetString();
                tp = iter->value.GetType();
                assert(true);
            }

            Value& sv = doc["hello"];
            tp = sv.GetType();
            Value& tv = doc["t"];
            tp = tv.GetType();
            assert(true);
        }
    }

    return 0;
}

std::string json_write()
{
    //const char json[] = " { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ";

    // 1. Parse a JSON string into DOM.
    //const char* json = "{\"project\":\"rapidjson\",\"stars\":10}";
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    writer.SetFormatOptions(kFormatSingleLineArray);
    //writer.SetIndent(' ', 1);
    if (writer.StartObject())
    {
        writer.String("hello");
        writer.String("world");

        writer.String("t");
        writer.Bool(true);

        writer.String("f");
        writer.Bool(false);

        writer.String("i");
        writer.Int(123);

        writer.String("a");
        writer.StartArray();
            writer.String("ss");
            writer.Int(1);
            writer.Int(2);
            writer.Int(3);

            writer.StartObject();

                writer.String("汉字");
                writer.String("是的");

                writer.String("杂交");
                writer.String("杂~!@#$%^&*()_+-=交\\';:,.<>?/");

            writer.EndObject();

        writer.EndArray();

        writer.EndObject();
    }

    return std::string(buffer.GetString());
}

void CModalDialogTest::OnBnClickedButton1()
{
    // GET
    //url_get();

    // POST
    //url_post();

    // JSON_PARSE
    //json_parse();

    // JSON_WRITE
    std::string json = json_write();
    url_post(json);
}

int CModalDialogTest::inc_id_ = 0;


// CModalDialogTest 消息处理程序

void CModalDialogTest::OnOK()
{
    CModalDialogTest modal(this);
    modal.DoModal();

    ::OutputDebugString((L"ModalDialog " + std::to_wstring(id_) + L"\r\n").c_str());
}

BOOL CModalDialogTest::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  在此添加额外的初始化
    SetWindowText((L"ModalTest " + std::to_wstring(id_)).c_str());

    return TRUE;  // return TRUE unless you set the focus to a control
    // 异常:  OCX 属性页应返回 FALSE
}

BOOL CModalDialogTest::DestroyWindow()
{
    return __super::DestroyWindow();
}
