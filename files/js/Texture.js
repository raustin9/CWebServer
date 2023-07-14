export default class Texture {
  GL = null;            // A webgl2 rendering context
  glTexture = null;     // This is a webgl texture
  textureImage = null;  // This is the image file that will be the rendered texture

  /**
   * 
   * @param {webgl Context} GL webgl rendering context to render and set values in
   * @param {String} texSource source of the texture
   * @param {JSON.Object} paramMap map of the parameters when creating the texture
   */
  constructor(GL, texSource) {
    this.GL = GL;
    this.glTexture = this.GL.createTexture();
    this.GL.bindTexture(this.GL.TEXTURE_2D, this.glTexture);

    const level = 0;
    const internalFormat = this.GL.RGBA;
    const width = 1;
    const height = 1;
    const border = 0;
    const srcFormat = this.GL.RGBA;
    const srcType = this.GL.UNSIGNED_BYTE;
    const pixel = new Uint8Array([0, 0, 255, 255]); // opaque blue
    this.GL.texImage2D(
      this.GL.TEXTURE_2D,
      level,
      internalFormat,
      width,
      height,
      border,
      srcFormat,
      srcType,
      pixel
    );

    this.textureImage = new Image();
    
    this.textureImage.onload = () => {
      this.GL.bindTexture(this.GL.TEXTURE_2D, this.glTexture);
      this.GL.texImage2D(
        this.GL.TEXTURE_2D,
        level,
        internalFormat,
        srcFormat,
        srcType,
        this.textureImage
      );
    }
    this.textureImage.src = texSource;


    this.GL.texParameteri(this.GL.TEXTURE_2D, this.GL.TEXTURE_WRAP_S, this.GL.CLAMP_TO_EDGE);
    this.GL.texParameteri(this.GL.TEXTURE_2D, this.GL.TEXTURE_WRAP_T, this.GL.CLAMP_TO_EDGE);
    this.GL.texParameteri(this.GL.TEXTURE_2D, this.GL.TEXTURE_MIN_FILTER, this.GL.LINEAR);

    this.GL.bindTexture(this.GL.TEXTURE_2D, null);
  }
}