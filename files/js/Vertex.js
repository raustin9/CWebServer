export  class VertexData {
  GL = null;
  bufferID = null;
  data = null;
  type = null;
  components = null;

  constructor(GL, data, type, perVertexComponents) {
    this.GL = GL;
    this.bufferID = this.GL.createBuffer();
    this.data = data;
    this.components = perVertexComponents;

    this.GL.bindBuffer(this.GL.ARRAY_BUFFER, this.bufferID);
    switch (type) {
      case this.GL.INT:
        this.GL.bufferData(this.GL.ARRAY_BUFFER, new Int32Array(this.data), this.GL.STATIC_DRAW);
        this.type = this.GL.INT;
        break;
      case this.GL.FLOAT:
        this.GL.bufferData(this.GL.ARRAY_BUFFER, new Float32Array(this.data), this.GL.STATIC_DRAW);
        this.type = this.GL.FLOAT;
        break;
      case this.GL.UNSIGNED_INT:
        this.GL.bufferData(this.GL.ARRAY_BUFFER, new Uint32Array(this.data), this.GL.STATIC_DRAW);
        this.type = this.GL.UNSIGNED_INT;
        break;
      default:
        throw `Uknown buffer tye: ${type}`;
    }
    this.GL.bindBuffer(this.GL.ARRAY_BUFFER, null);
  }

  BindAndEnable(vertexAttributeLocation) {
    this.GL.bindBuffer(this.GL.ARRAY_BUFFER, this.bufferID);
    const numComponents = this.components;
    const type = this.type;
    const normalize = false;
    const stride = 0;
    const offset = 0;

    this.GL.vertexAttribPointer(
      vertexAttributeLocation,
      numComponents,
      type,
      normalize,
      stride,
      offset
    );
    this.GL.enableVertexAttribArray(vertexAttributeLocation);
  }
}

export class ElementData {
  GL = null;
  bufferID = null;
  data = null;
  count = null;
  type = null;

  constructor(GL, data) {
    this.GL = GL;
    this.bufferID = this.GL.createBuffer();
    this.data = data;
    this.count = data.length;
    this.type = this.GL.UNSIGNED_SHORT;

    this.GL.bindBuffer(this.GL.ELEMENT_ARRAY_BUFFER, this.bufferID);
    this.GL.bufferData(this.GL.ELEMENT_ARRAY_BUFFER, new Uint16Array(data), this.GL.STATIC_DRAW);
    this.GL.bindBuffer(this.GL.ELEMENT_ARRAY_BUFFER, null);
  }

  BindAndEnable() {
    this.GL.bindBuffer(this.GL.ELEMENT_ARRAY_BUFFER, this.bufferID);
  }
}