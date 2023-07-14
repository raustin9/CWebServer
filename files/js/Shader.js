export default class Shader {
  GL = null;
  shaderProgram = null;
  vertSource = null;
  fragSource = null;
  vShader = null;
  fShader = null;

  constructor(GL, vsrc, fsrc) {
    // LOAD SOURCES FOR FRAGMENT AND VERTEX SHADERS
    this.GL = GL;
    this.vertSource = vsrc;
    this.fragSource = fsrc;
    this.vertexShader   = this.LoadShader(GL, GL.VERTEX_SHADER, this.vertSource);
    this.fragmentShader = this.LoadShader(GL, GL.FRAGMENT_SHADER, this.fragSource);

    // CREATE PROGRAM AND ATTACH SHADERS
    this.shaderProgram = GL.createProgram();

    GL.attachShader(this.shaderProgram, this.vertexShader);
    GL.attachShader(this.shaderProgram, this.fragmentShader);

    GL.linkProgram(this.shaderProgram);

    // CHECK FOR LINK ERROR
    if (!GL.getProgramParameter(this.shaderProgram, GL.LINK_STATUS)) {
      alert(
        `Unable to initialize the shader program: ${GL.getProgramInfoLog(
          this.shaderProgram
        )}`
      );
      return null;
    }
  }

  /**
   * 
   * @param {WEBGL CONTExt} GL the context for webgl
   * @param {GLenum} type type of shader
   * @param {String} src source of the shader
   * @returns {WebGLShader} returns the compiled shader
   */
  LoadShader(GL, type, src) {
    const shader = GL.createShader(type);

    GL.shaderSource(shader, src);

    GL.compileShader(shader);

    if (!GL.getShaderParameter(shader, GL.COMPILE_STATUS)) {
      alert(`
        ERROR IN SHADER: ${GL.getShaderInfoLog(shader)}
      `);
      GL.deleteShader(shader);
      return null;
    }

    return shader;
  }

  GetAttributeLocations(attributeNames) {
    let attributeLocations = {};

    attributeNames.forEach(attributeName => {
      attributeLocations[attributeName] = this.GL.getAttribLocation(this.shaderProgram, attributeName);
    });

    return attributeLocations;
  }

  GetUniformLocations(uniformNames) {
    let uniformLocations = {};

    uniformNames.forEach(uniformName => {
      uniformLocations[uniformName] = this.GL.getUniformLocation(this.shaderProgram, uniformName);
    });
    
    return uniformLocations;
  }
}