#include <boost/scoped_ptr.hpp>

#include "imagebuffer.h"

OIIO_NAMESPACE_USING;

ImageBuffer::ImageBuffer(const std::string &filename)
    : ImageBuf(filename),
      m_image_valid(false)
{
    std::cout << filename.c_str() << std::endl;
    bool ok = ImageBuf::init_spec (filename, 0, 0);
    if (ok && m_file_dataformat.basetype == TypeDesc::UNKNOWN) {
        m_file_dataformat = m_spec.format;
    }
    //m_filename = filename;
    //m_name = boost::filesystem::path(filename).leaf();
}

ImageBuffer::~ImageBuffer ()
{
}

bool
ImageBuffer::init_spec(const std::string &filename, int subimage, int miplevel)
{
    bool ok = ImageBuf::init_spec (filename, 0, 0);
    if (ok && m_file_dataformat.basetype == TypeDesc::UNKNOWN) {
        m_file_dataformat = m_spec.format;
    }
    return ok;
}



bool
ImageBuffer::read (int subimage, int miplevel, bool force, TypeDesc format,
               ProgressCallback progress_callback,
               void *progress_callback_data, bool secondary_data)
{
    // Don't read if we already have it in memory, unless force is true.
    // FIXME: should we also check the time on the file to see if it's
    // been updated since we last loaded?
    if (m_image_valid && !force){
        std::cout << "Reading Cached Image!" << std::endl;
        return true;
    }

    m_image_valid = ImageBuf::read (subimage, miplevel, force, format,
                                    progress_callback, progress_callback_data);

    populate_channels();

    return m_image_valid;
}

bool ImageBuffer::populate_channels(bool force_rgba){

    if (!m_image_valid)
        return 1;
    m_layernames.clear();
    m_layerchannels.clear();
    // Reserve first channel for RGBA
    m_layernames.resize(1, "");
    m_layerchannels.resize(1);
    unsigned index = 0;
    std::vector<std::string> channelnames = m_spec.channelnames;
    for(std::vector<std::string>::iterator it = channelnames.begin(); it<channelnames.end(); it++, index++){
        std::string channelname = *it;
        if(channelname=="R" || channelname=="G" ||channelname=="B" || channelname=="A"){
            std::cout << "Channels: " << *it << std::endl;
            m_layernames[0] += channelname;
            m_layerchannels[0].push_back(index);
        }else{
            std::string layername = channelname.substr(0,channelname.find_last_of("."));
            std::vector<std::string>::iterator location = std::find(m_layernames.begin(), m_layernames.end(), layername);
            if(location==m_layernames.end()){
                std::cout << layername << std::endl;
                m_layernames.push_back(layername);
                std::vector<int> c_chan;
                m_layerchannels.push_back(c_chan);
            }
            m_layerchannels.back().push_back(index);
        }
    }
    if(m_layernames.front()==""){ // Clear the reserved RGBA layer if empty.
        m_layernames.erase(m_layernames.begin());
        m_layerchannels.erase(m_layerchannels.begin());
    }

    for(int l=0; l<m_layernames.size();l++){
        std::cout << "Layers: " << m_layernames[l] << ", Channels: [";
        for(int c=0; c<m_layerchannels[l].size(); c++){
            std::cout << m_layerchannels[l][c];
            if(c<m_layerchannels[l].size()-1)
                std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    }
    /*
    std::vector< std::vector<int> >::iterator channel_it = m_layerchannels.begin();
    for(std::vector<std::string>::iterator layer_it = m_layernames.begin(); layer_it<m_layernames.end(); layer_it++, channel_it++){
        std::cout << "Layer: " << *layer_it << " [";
        for(int i = 0; i < channel_it->size(); i++){
            std::cout << i << ",";
        }
        std::cout << "]" << std::endl;
    }
    */
    return 0;
}

void
ImageBuffer::invalidate ()
{
    m_pixels_valid = false;
    m_image_valid = false;
    if (m_imagecache) {
        m_imagecache->invalidate(m_name);
    }
}

template<typename S, typename D>
static inline void
copy_pixel_channels_ (const ImageBuf &buf, int xbegin, int xend,
              int ybegin, int yend, int chbegin, int chend, D *r)
{
    int w = (xend-xbegin);
    int nc = (chend-chbegin);
    for (ImageBuf::ConstIterator<S,D> p (buf, xbegin, xend, ybegin, yend);
         p.valid(); ++p) {
        imagesize_t offset = ((p.y()-ybegin)*w + (p.x()-xbegin)) * nc;
        for (int c = 0;  c < nc;  ++c)
            r[offset+c] = p[chbegin+c];
    }
}

template<typename D>
static inline bool
copy_pixel_channels (const ImageBuf &buf, int xbegin, int xend,
              int ybegin, int yend, int chbegin, int chend, D *r)
{
    // Caveat: serious hack here.  To avoid duplicating code, use a
    // #define.  Furthermore, exploit the CType<> template to construct
    // the right C data type for the given BASETYPE.
#define TYPECASE(B)                                                     \
    case B : copy_pixel_channels_<CType<B>::type,D>(buf, xbegin, xend, ybegin, yend, chbegin, chend, (D *)r); return true

    switch (buf.spec().format.basetype) {
        TYPECASE (TypeDesc::UINT8);
        TYPECASE (TypeDesc::INT8);
        TYPECASE (TypeDesc::UINT16);
        TYPECASE (TypeDesc::INT16);
        TYPECASE (TypeDesc::UINT);
        TYPECASE (TypeDesc::INT);
        //TYPECASE (TypeDesc::HALF);
        TYPECASE (TypeDesc::FLOAT);
        TYPECASE (TypeDesc::DOUBLE);
    }
    return false;
#undef TYPECASE
}

bool
ImageBuffer::copy_pixel_channels (int xbegin, int xend, int ybegin, int yend,
                       int chbegin, int chend, TypeDesc format, void *result) const
{
    if (chend > nchannels())
        return false;

    // Fancy method -- for each possible base type that the user
    // wants for a destination type, call a template specialization.
    switch (format.basetype) {
    case TypeDesc::UINT8 :
        ::copy_pixel_channels<unsigned char> (*this, xbegin, xend, ybegin, yend, chbegin, chend, (unsigned char *)result);
        break;
    case TypeDesc::INT8:
        ::copy_pixel_channels<char> (*this, xbegin, xend, ybegin, yend, chbegin, chend, (char *)result);
        break;
    case TypeDesc::UINT16 :
        ::copy_pixel_channels<unsigned short> (*this, xbegin, xend, ybegin, yend, chbegin, chend, (unsigned short *)result);
        break;
    case TypeDesc::INT16 :
        ::copy_pixel_channels<short> (*this, xbegin, xend, ybegin, yend, chbegin, chend, (short *)result);
        break;
    case TypeDesc::UINT :
        ::copy_pixel_channels<unsigned int> (*this, xbegin, xend, ybegin, yend, chbegin, chend, (unsigned int *)result);
        break;
    case TypeDesc::INT :
        ::copy_pixel_channels<int> (*this, xbegin, xend, ybegin, yend, chbegin, chend, (int *)result);
        break;
    //case TypeDesc::HALF :
    //    ::copy_pixel_channels<half> (*this, xbegin, xend, ybegin, yend, chbegin, chend, (half *)result);
    //    break;
    case TypeDesc::FLOAT :
        ::copy_pixel_channels<float> (*this, xbegin, xend, ybegin, yend, chbegin, chend, (float *)result);
        break;
    case TypeDesc::DOUBLE :
        ::copy_pixel_channels<double> (*this, xbegin, xend, ybegin, yend, chbegin, chend, (double *)result);
        break;
    default:
        return false;
    }
    return true;
}
