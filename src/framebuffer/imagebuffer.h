#ifndef IMAGEBUFFER_H
#define IMAGEBUFFER_H

#include "imageio.h"
#include "imagebuf.h"

OIIO_NAMESPACE_USING;

class ImageBuffer : public ImageBuf {
public:
    ImageBuffer(const std::string &filename);
    virtual ~ImageBuffer ();
    void InitializeBuffer();

    /// Read the image into ram.
    /// If secondary buffer is true, and the format is UINT8, then a secondary
    /// buffer will be created and the apply_corrections(), and
    /// select_channel() methods will work.
    /// Also, scanline will return a pointer to that buffer instead of the read
    /// buffer.
    virtual bool read (int subimage=0, int miplevel=0,
                       bool force=false, TypeDesc format = TypeDesc::UNKNOWN,
                       ProgressCallback progress_callback=NULL,
                       void *progress_callback_data=NULL, bool secondary_buffer=false);
    virtual bool init_spec (const std::string &filename,
                            int subimage, int miplevel);

    bool populate_channels(bool force_rgba=true);

    ustring getName(){return m_name;}
    void invalidate ();

    /// Can we read the pixels of this image already?
    ///
    bool image_valid() const { return m_image_valid; }

    bool copy_pixel_channels  (int xbegin, int xend, int ybegin, int yend,
                           int chbegin, int chend, TypeDesc format, void *result) const;
private:
    TypeDesc m_file_dataformat;
    bool m_image_valid;
    std::vector<std::string> m_layernames;  // Names for each layer
    std::vector< std::vector<int> > m_layerchannels; // Channel indices for each layer

};

#endif // IMAGEBUFFER_H
