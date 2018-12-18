#ifndef OPENCV_CVX_TEXT_HPP_
#define OPENCV_CVX_TEXT_HPP_

// source from: http://www.opencv.org.cn/forum.php?mod=viewthread&tid=2083&extra=&page=1
// 支持OpenCV中文汉字输入

#include <wchar.h>
#include <assert.h>
#include <locale.h>
#include <ctype.h>
#include <cmath>
#include <exception>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftcache.h>

#include <opencv2/opencv.hpp>


typedef struct MyFaceRec_ {
    const char* file_path;
    int face_index;
} MyFaceRec, *MyFace;


FT_Error my_face_requester(FTC_FaceID face_id, FT_Library library, FT_Pointer req_data, FT_Face* aface) {
    MyFace face = (MyFace)face_id; // simple typecase
    return FT_New_Face(library, face->file_path, face->face_index, aface);
}


class CvxText {
public:
    /**
    * 装载字库文件
    */
    CvxText(const char* freeType) {
        assert(freeType != NULL);
        m_myface.file_path = freeType;
        m_myface.face_index = 0;
        m_myfaceId = (FTC_FaceID)&m_myface;

        // 打开字库文件, 创建一个字体
        if (FT_Init_FreeType(&m_library)) {
            throw std::runtime_error("FT_Init_FreeType exception");
        }
        if (FTC_Manager_New(m_library, 0, 0, 0, my_face_requester, nullptr, &m_cacheManager)) {
            throw std::runtime_error("FTC_Manager_New exception");
        }
        if (FTC_CMapCache_New(m_cacheManager, &m_mapCache)) {
            throw std::runtime_error("FTC_CMapCache_New exception");
        }
        if (FTC_SBitCache_New(m_cacheManager, &m_sbitCache)) {
            throw std::runtime_error("FTC_SBitCache_New exception");
        }
        if (FTC_Manager_LookupFace(m_cacheManager, m_myfaceId, &m_face)) {
            throw std::runtime_error("FTC_Manager_LookupFace exception");
        }

        // 设置字体输出参数
        restoreFont();

        // 设置C语言的字符集环境
        setlocale(LC_ALL, "");
    }

    virtual ~CvxText() {
        FTC_Manager_RemoveFaceID(m_cacheManager, m_myfaceId);
        FTC_Manager_Reset(m_cacheManager);
        FTC_Manager_Done(m_cacheManager);
        FT_Done_FreeType(m_library);
    }

    /**
    * 获取字体.目前有些参数尚不支持.
    *
    * \param font        字体类型, 目前不支持
    * \param size        字体大小/空白比例/间隔比例/旋转角度
    * \param underline   下画线
    * \param diaphaneity 透明度
    *
    * \sa setFont, restoreFont
    */
    void getFont(int* type, cv::Scalar* size = nullptr, bool* underline = nullptr, float* diaphaneity = nullptr) {
        if (type) *type = m_fontType;
        if (size) *size = m_fontSize;
        if (underline) *underline = m_fontUnderline;
        if (diaphaneity) *diaphaneity = m_fontDiaphaneity;
    }

    /**
    * 设置字体.目前有些参数尚不支持.
    *
    * \param font        字体类型, 目前不支持
    * \param size        字体大小/空白比例/间隔比例/旋转角度
    * \param underline   下画线
    * \param diaphaneity 透明度
    *
    * \sa getFont, restoreFont
    */
    void setFont(int* type, cv::Scalar* size = nullptr, bool* underline = nullptr, float* diaphaneity = nullptr) {
        // 参数合法性检查
        if (type) {
            m_fontType = *type;
        }
        if (size) {
            m_fontSize.val[0] = std::fabs(size->val[0]);
            m_fontSize.val[1] = std::fabs(size->val[1]);
            m_fontSize.val[2] = std::fabs(size->val[2]);
            m_fontSize.val[3] = std::fabs(size->val[3]);
        }
        if (underline) {
            m_fontUnderline = *underline;
        }
        if (diaphaneity) {
            m_fontDiaphaneity = *diaphaneity;
        }

        m_scaler.height = m_fontSize.val[0];
        m_scaler.width = m_fontSize.val[0];

        FTC_Manager_LookupSize(m_cacheManager, &m_scaler, nullptr);
    }

    /**
    * 恢复原始的字体设置.
    *
    * \sa getFont, setFont
    */
    void restoreFont() {
        m_fontType = 0;            // 字体类型(不支持)
        m_fontUnderline = false;   // 下画线(不支持)
        m_fontDiaphaneity = 1.0;   // 色彩比例(可产生透明效果)

        m_scaler.face_id = m_myfaceId;
        m_scaler.width = 20;
        m_scaler.height = 20;
        m_scaler.pixel = 1;
        m_scaler.x_res = 0;
        m_scaler.y_res = 0;

        // 设置字符大小
        FTC_Manager_LookupSize(m_cacheManager, &m_scaler, nullptr);
    }

    /**
    * 输出汉字(颜色默认为黑色).遇到不能输出的字符将停止.
    *
    * \param img  输出的影象
    * \param text 文本内容
    * \param pos  文本位置
    *
    * \return 返回成功输出的字符长度，失败返回-1.
    */
    int putText(cv::Mat& img, const char* text, cv::Point pos) {
        return putText(img, text, pos, CV_RGB(255, 255, 255));
    }

    /**
    * 输出汉字(颜色默认为黑色).遇到不能输出的字符将停止.
    *
    * \param img  输出的影象
    * \param text 文本内容
    * \param pos  文本位置
    *
    * \return 返回成功输出的字符长度，失败返回-1.
    */
    int putText(cv::Mat& img, const wchar_t* text, cv::Point pos) {
        return putText(img, text, pos, CV_RGB(255, 255, 255));
    }

    /**
    * 输出汉字.遇到不能输出的字符将停止.
    *
    * \param img   输出的影象
    * \param text  文本内容
    * \param pos   文本位置
    * \param color 文本颜色
    *
    * \return 返回成功输出的字符长度，失败返回-1.
    */
    int putText(cv::Mat& img, const char* text, cv::Point pos, cv::Scalar color) {
        if (img.data == nullptr) return -1;
        if (text == nullptr) return -1;

        int i;
        for (i = 0; text[i] != '\0'; ++i) {
            wchar_t wc = text[i];

            // 解析双字节符号
            if (!isascii(wc)) mbtowc(&wc, &text[i++], 2);

            // 输出当前的字符
            putWChar(img, wc, pos, color);
        }

        return i;
    }

    /**
    * 输出汉字.遇到不能输出的字符将停止.
    *
    * \param img   输出的影象
    * \param text  文本内容
    * \param pos   文本位置
    * \param color 文本颜色
    *
    * \return 返回成功输出的字符长度，失败返回-1.
    */
    int putText(cv::Mat& img, const wchar_t* text, cv::Point pos, cv::Scalar color) {
        if (img.data == nullptr) return -1;
        if (text == nullptr) return -1;

        int i;
        for (i = 0; text[i] != '\0'; ++i) {
            // 输出当前的字符
            putWChar(img, text[i], pos, color);
        }

        return i;
    }

private:
    // 禁止copy
    CvxText & operator=(const CvxText&);
    // 输出当前字符, 更新m_pos位置
    void putWChar(cv::Mat& img, wchar_t wc, cv::Point& pos, cv::Scalar color) {
        FTC_SBit bitmap;

        FT_UInt glyph_idx = FTC_CMapCache_Lookup(m_mapCache, m_scaler.face_id, -1, (FT_UInt32)wc);
        if (0 == glyph_idx) {
            return;
        }
        FTC_SBitCache_LookupScaler(m_sbitCache, &m_scaler, FT_LOAD_MONOCHROME, glyph_idx, &bitmap, nullptr);

        // 行列数
        int rows = bitmap->height;
        int cols = bitmap->width;

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                int off = i * bitmap->pitch + j / 8;

                if (bitmap->buffer[off] & (0xC0 >> (j % 8))) {
                    int r = pos.y - (rows - 1 - i);
                    int c = pos.x + j;

                    if (r >= 0 && r < img.rows && c >= 0 && c < img.cols) {
                        cv::Vec3b pixel = img.at<cv::Vec3b>(cv::Point(c, r));
                        cv::Scalar scalar = cv::Scalar(pixel.val[0], pixel.val[1], pixel.val[2]);

                        // 进行色彩融合
                        float p = m_fontDiaphaneity;
                        for (int k = 0; k < 4; ++k) {
                            scalar.val[k] = scalar.val[k] * (1 - p) + color.val[k] * p;
                        }

                        img.at<cv::Vec3b>(cv::Point(c, r))[0] = (unsigned char)(scalar.val[0]);
                        img.at<cv::Vec3b>(cv::Point(c, r))[1] = (unsigned char)(scalar.val[1]);
                        img.at<cv::Vec3b>(cv::Point(c, r))[2] = (unsigned char)(scalar.val[2]);
                    }
                }
            }
        }

        // 修改下一个字的输出位置
        double space = m_fontSize.val[0] * m_fontSize.val[1];
        double sep = m_fontSize.val[0] * m_fontSize.val[2];

        pos.x += (int)((cols ? cols : space) + sep);
    }


private:
    FT_Library   m_library;   // 字库
    FT_Face      m_face;      // 字体

    //字体缓存
    MyFaceRec     m_myface;
    FTC_FaceID    m_myfaceId;
    FTC_Manager   m_cacheManager;
    FTC_CMapCache m_mapCache;
    FTC_SBitCache m_sbitCache;
    FTC_ScalerRec m_scaler;

    // 默认的字体输出参数
    int         m_fontType;
    cv::Scalar  m_fontSize;
    bool        m_fontUnderline;
    float       m_fontDiaphaneity;
};

#endif // OPENCV_CVX_TEXT_HPP_
