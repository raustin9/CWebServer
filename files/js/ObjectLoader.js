export default class OBJData {
  constructor(fileContents) {
    this.fileContents = fileContents;
    this.parse(this.fileContents);
  }

  result = {
    models: [],
    materialLibraries: []
  };
  currentMaterial = '';
  currentGroup = '';
  smoothingGroup = 0;
  fileContents = null;

  parse() {

    const stripComment = (lineString) => {
      const commentIndex = lineString.indexOf('#');
      if (commentIndex > -1) { return lineString.substring(0, commentIndex); }
      return lineString;
    };

    const lines = this.fileContents.split('\n');
    for (let i = 0; i < lines.length; i += 1) {
      const line = stripComment(lines[i]);

      const lineItems = line.replace(/\s\s+/g, ' ').trim().split(' ');
      if (line.length <= 0) continue;
      switch (lineItems[0].toLowerCase()) {
        case 'o': // Start A New Model
          this.parseObject(lineItems);
          break;
        case 'g': // Start a new polygon group
          this.parseGroup(lineItems);
          break;
        case 'v': // Define a vertex for the current model
          this.parseVertexCoords(lineItems);
          break;
        case 'vt': // Texture Coords
          this.parseTextureCoords(lineItems);
          break;
        case 'vn': // Define a vertex normal for the current model
          this.parseVertexNormal(lineItems);
          break;
        case 's': // Smooth shading statement
          this.parseSmoothShadingStatement(lineItems);
          break;
        case 'f': // Define a Face/Polygon
          this.parsePolygon(lineItems);
          break;
        case 'mtllib': // Reference to a material library file (.mtl)
          this.parseMtlLib(lineItems);
          break;
        case 'usemtl': // Sets the current material to be applied to polygons defined from this point forward
          this.parseUseMtl(lineItems);
          break;
        case '#':
          break;
        default:
          console.warn(`Unhandled obj statement at line #${i}: ${line}`);
          break;
      }
    }

    return this.result;
  }

  currentModel() {
    if (this.result.models.length == 0) {
      this.result.models.push({
        name: this.defaultModelName,
        vertices: [],
        textureCoords: [],
        vertexNormals: [],
        faces: []
      });
      this.currentGroup = '';
      this.smoothingGroup = 0;
    }

    return this.result.models[this.result.models.length - 1];
  }

  getModelAtIndex(index) {
    return this.result.models[index];
  }

  /**
   * getIndexableDataFromModelAtIndex - generates indexable raw geometry data for the model at specified index
   * @param {Number} index the index of the model you are interested in (multiple models may exist in one obj file) 
   */
  getIndexableDataFromModelAtIndex(index) {
    /*
    There is no guarantee this function works for 100% of obj models.
    This assumes for each vertex there is exactly one normal, one position, one texture coord.
    Some models do not abide by this, such as discrete models.
    */
    const model = this.result.models[index];
    const faces = model.faces;
    const vertices = model.vertices;
    const textureCoords = model.textureCoords;
    const vertexNormals = model.vertexNormals;

    let indexData = [];
    faces.forEach(face => {
      // A face can have 3+ vertices.
      // Since we want triangles, we turn the face into a fan of triangles.
      // http://docs.safe.com/fme/2017.1/html/FME_Desktop_Documentation/FME_Workbench/!FME_Geometry/IFMETriangleFan.htm
      let vertsOnFace = face.vertices;
      let initialVert = vertsOnFace[0];
      for (let i = 1; i < vertsOnFace.length - 1; ++i){
        let triangle = [initialVert, vertsOnFace[i], vertsOnFace[i + 1]];
        triangle.forEach(triangleVert => {
          indexData.push(triangleVert.vertexIndex - 1);
        });
      }
    });

    let textureData = [];
    textureCoords.forEach(coord => {
      textureData.push(coord.u);
      textureData.push(coord.v);
    });

    let normalData = [];
    vertexNormals.forEach(normal => {
      normalData.push(normal.x);
      normalData.push(normal.y);
      normalData.push(normal.z);
    });

    let vertexData = [];
    vertices.forEach(vertex => {
      vertexData.push(vertex.x);
      vertexData.push(vertex.y);
      vertexData.push(vertex.z);
    });

    return {
      indices: indexData,
      uvs: textureData,
      normals: normalData,
      vertices: vertexData
    }
  }

  
  /**
   * getFlattenedDataFromModelAtIndex - generates flattened geometry data, prefer the indexable method if model is not discrete
   * @param {Number} index the index of the model you are interested in (multiple models may exist in one obj file) 
   */
  getFlattenedDataFromModelAtIndex(index) {
    const model = this.result.models[index];
    const faces = model.faces;
    const vertices = model.vertices;
    const textureCoords = model.textureCoords;
    const vertexNormals = model.vertexNormals;

    let minX, minY, minZ;
    let maxX, maxY, maxZ;

    minX = vertices[0].x;
    minY = vertices[0].y;
    minZ = vertices[0].z;

    maxX = vertices[0].x;
    maxY = vertices[0].y;
    maxZ = vertices[0].z;

    vertices.forEach(vertex => {
      if (vertex.x < minX) minX = vertex.x;
      if (vertex.y < minY) minY = vertex.y;
      if (vertex.z < minZ) minZ = vertex.z;

      if (vertex.x > maxX) maxX = vertex.x;
      if (vertex.y > maxY) maxY = vertex.y;
      if (vertex.z > maxZ) maxZ = vertex.z;
    });

    let minvals = [minX, minY, minZ];
    let maxvals = [maxX, maxY, maxZ];

    let diffvals = [
      maxvals[0] - minvals[0],
      maxvals[1] - minvals[1],
      maxvals[2] - minvals[2]
    ];

    let valmagnitude = Math.sqrt(Math.pow(diffvals[0],2) + Math.pow(diffvals[1],2) + Math.pow(diffvals[2],2));
    let magadjusted = [
      diffvals[0] / valmagnitude * 3,
      diffvals[1] / valmagnitude * 3,
      diffvals[2] / valmagnitude * 3
    ];

    let scales = [
      magadjusted[0] / diffvals[0],
      magadjusted[1] / diffvals[1],
      magadjusted[2] / diffvals[2]
    ];

    vertices.forEach(vertex => {
      vertex.x *= scales[0];
      vertex.y *= scales[1];
      vertex.z *= scales[2];
    });

    let textureData = [];
    let normalData = [];
    let vertexData = [];

    // This is just for the wireframe shader, feel free to remove this information if not necessary
    // I am only including it here for a cheap wireframe effect.
    let barycentricCoords = [];
    let barycentricValues = [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]];
    faces.forEach(face => {
      // A face can have 3+ vertices.
      // Since we want triangles, we turn the face into a fan of triangles.
      // http://docs.safe.com/fme/2017.1/html/FME_Desktop_Documentation/FME_Workbench/!FME_Geometry/IFMETriangleFan.htm
      let vertsOnFace = face.vertices;
      let initialVert = vertsOnFace[0];

      for (let i = 1; i < vertsOnFace.length - 1; ++i){
        let triangle = [initialVert, vertsOnFace[i], vertsOnFace[i + 1]];

        let A = [
          vertices[triangle[1].vertexIndex-1].x - vertices[triangle[0].vertexIndex-1].x,
          vertices[triangle[1].vertexIndex-1].y - vertices[triangle[0].vertexIndex-1].y,
          vertices[triangle[1].vertexIndex-1].z - vertices[triangle[0].vertexIndex-1].z
        ];
        let B = [
          vertices[triangle[2].vertexIndex-1].x - vertices[triangle[1].vertexIndex-1].x,
          vertices[triangle[2].vertexIndex-1].y - vertices[triangle[1].vertexIndex-1].y,
          vertices[triangle[2].vertexIndex-1].z - vertices[triangle[1].vertexIndex-1].z
        ];

        // find the cross product between the two vectors
        let cross = [ A[1]*B[2] - A[2]*B[1] , A[2]*B[0] - A[0]*B[2] , A[0]*B[1] - A[1]*B[0] ];
        let magCross = Math.sqrt( cross[0]*cross[0] + cross[1]*cross[1] + cross[2]*cross[2] );
        cross = [cross[0] / magCross , cross[1] / magCross , cross[2] / magCross];

        triangle.forEach((triangleVert, index) => {
          // Obj models are not zero index, so we subtract 1 from the indicated indices
          vertexData.push(vertices[triangleVert.vertexIndex - 1].x);
          vertexData.push(vertices[triangleVert.vertexIndex - 1].y);
          vertexData.push(vertices[triangleVert.vertexIndex - 1].z);

          if(textureCoords[triangleVert.textureCoordsIndex - 1]) {
            textureData.push(textureCoords[triangleVert.textureCoordsIndex - 1].u);
            textureData.push(textureCoords[triangleVert.textureCoordsIndex - 1].v);
          }

          if(vertexNormals[triangleVert.vertexNormalIndex - 1]) {
            normalData.push(vertexNormals[triangleVert.vertexNormalIndex - 1].x);
            normalData.push(vertexNormals[triangleVert.vertexNormalIndex - 1].y);
            normalData.push(vertexNormals[triangleVert.vertexNormalIndex - 1].z);
          } else {
            // no normal data we need to make it
            for (let n = 0; n < cross.length; n++) {
              normalData.push(cross[n]);
            }
          }

          barycentricCoords.push(barycentricValues[index][0]);
          barycentricCoords.push(barycentricValues[index][1]);
          barycentricCoords.push(barycentricValues[index][2]);
        });
      }
    });

    // if (normalData.length < 1) console.warn("No normal data loaded for model.");
    if (vertexData.length < 1) console.warn("No vertex data loaded for model.");
    // if (textureData.length < 1) console.warn("No texture data loaded for model.");
    return {
      uvs: textureData,
      normals: normalData,
      vertices: vertexData,
      barycentricCoords: barycentricCoords,
    }
  }

  parseObject(lineItems) {
    const modelName = lineItems.length >= 2 ? lineItems[1] : this.defaultModelName;
    this.result.models.push({
      name: modelName,
      vertices: [],
      textureCoords: [],
      vertexNormals: [],
      faces: []
    });
    this.currentGroup = '';
    this.smoothingGroup = 0;
  }

  parseGroup(lineItems) {
    if (lineItems.length != 2) { throw 'Group statements must have exactly 1 argument (eg. g group_1)'; }

    this.currentGroup = lineItems[1];
  }

  parseVertexCoords(lineItems) {
    const x = lineItems.length >= 2 ? parseFloat(lineItems[1]) : 0.0;
    const y = lineItems.length >= 3 ? parseFloat(lineItems[2]) : 0.0;
    const z = lineItems.length >= 4 ? parseFloat(lineItems[3]) : 0.0;

    this.currentModel().vertices.push({ x, y, z });
  }

  parseTextureCoords(lineItems) {
    const u = lineItems.length >= 2 ? parseFloat(lineItems[1]) : 0.0;
    const v = lineItems.length >= 3 ? parseFloat(lineItems[2]) : 0.0;
    const w = lineItems.length >= 4 ? parseFloat(lineItems[3]) : 0.0;

    if (lineItems.length >= 4)
      this.currentModel().textureCoords.push({ u, v, w });
    else
      this.currentModel().textureCoords.push({ u, v });
  }

  parseVertexNormal(lineItems) {
    const x = lineItems.length >= 2 ? parseFloat(lineItems[1]) : 0.0;
    const y = lineItems.length >= 3 ? parseFloat(lineItems[2]) : 0.0;
    const z = lineItems.length >= 4 ? parseFloat(lineItems[3]) : 0.0;

    this.currentModel().vertexNormals.push({ x, y, z });
  }

  parsePolygon(lineItems) {
    const totalVertices = (lineItems.length - 1);
    if (totalVertices < 3) { throw (`Face statement has less than 3 vertices${this.filePath}${this.lineNumber}`); }

    const face = {
      material: this.currentMaterial,
      group: this.currentGroup,
      smoothingGroup: this.smoothingGroup,
      vertices: []
    };

    for (let i = 0; i < totalVertices; i += 1) {
      const vertexString = lineItems[i + 1];
      const vertexValues = vertexString.split('/');

      if (vertexValues.length < 1 || vertexValues.length > 3) { throw (`Too many values (separated by /) for a single vertex${this.filePath}${this.lineNumber}`); }

      let vertexIndex = 0;
      let textureCoordsIndex = 0;
      let vertexNormalIndex = 0;
      vertexIndex = parseInt(vertexValues[0]);
      if (vertexValues.length > 1 && (!vertexValues[1] == '')) { textureCoordsIndex = parseInt(vertexValues[1]); }
      if (vertexValues.length > 2) { vertexNormalIndex = parseInt(vertexValues[2]); }

      if (vertexIndex == 0) { throw 'Faces uses invalid vertex index of 0'; }

      // Negative vertex indices refer to the nth last defined vertex
      // convert these to postive indices for simplicity
      if (vertexIndex < 0) { vertexIndex = this.currentModel().vertices.length + 1 + vertexIndex; }

      face.vertices.push({
        vertexIndex,
        textureCoordsIndex,
        vertexNormalIndex
      });
    }
    this.currentModel().faces.push(face);
  }

  parseMtlLib(lineItems) {
    if (lineItems.length >= 2) { this.result.materialLibraries.push(lineItems[1]); }
  }

  parseUseMtl(lineItems) {
    if (lineItems.length >= 2) { this.currentMaterial = lineItems[1]; }
  }

  parseSmoothShadingStatement(lineItems) {
    if (lineItems.length != 2) { throw 'Smoothing group statements must have exactly 1 argument (eg. s <number|off>)'; }

    const groupNumber = (lineItems[1].toLowerCase() == 'off') ? 0 : parseInt(lineItems[1]);
    this.smoothingGroup = groupNumber;
  }
}
