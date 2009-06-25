/**
 * Functions for working with HL_Node's.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-06-11
 */
#include "hlhdf.h"
#include "hlhdf_private.h"
#include "hlhdf_defines_private.h"
#include "hlhdf_node_private.h"
#include "hlhdf_debug.h"
#include <string.h>
#include <stdlib.h>

/*@{ Structs */
/**
 * Represents a HDF5 object/attribute/reference/...
 * @ingroup hlhdf_c_apis
 */
struct _HL_Node {
   HL_Type type;               /**< The type of this node */
   char* name;                 /**< The name of this node */
   int ndims;                  /**< Number of dimensions if this node is represented by a HL_Type#ATTRIBUTE_ID or HL_Type#TYPE_ID*/
   hsize_t* dims;              /**< The dimension size */
   unsigned char* data;        /**< The data in fixed-type format */
   unsigned char* rawdata;     /**< Unconverted data, exactly as read from the file */
   HL_FormatSpecifier format;  /**< @ref ValidFormatSpecifiers "Format specifier" */
   hid_t typeId;               /**< HDF5 type identifier */
   size_t dSize;               /**< Size for data (fixed type) */
   size_t rdSize;              /**< Size for rawdata */
   HL_DataType dataType;       /**< Type of data */
   hid_t hdfId;                /**< The hdf id that this node represents (used internally)*/
   HL_NodeMark mark;           /**< Current state of this node */
   HL_CompoundTypeDescription* compoundDescription; /**< The compound type description if this is a TYPE node*/
   HL_Compression* compression; /**< Compression settings for this node */
};

/*@{ End of Structs */

/*@{ Static functions */

static HL_Node* newHL_NodeWithType(const char* name, HL_Type type)
{
  HL_Node* retv = NULL;
  HL_SPEWDEBUG0("ENTER: newHL_NodeWithType");
  if (!(retv = newHL_Node(name))) {
    HL_ERROR0("Failed to allocate HL_Node item");
    goto fail;
  }
  setHL_NodeType(retv, type);
fail:
  HL_SPEWDEBUG0("EXIT: newHL_NodeWithType");
  return retv;
}

static hid_t HLNode_createStringType(size_t length)
{
  hid_t type;
  HL_SPEWDEBUG0("ENTER: HLNode_createStringType");

  type = H5Tcopy(H5T_C_S1);
  H5Tset_size(type, length);

  HL_SPEWDEBUG0("EXIT: HLNode_createStringType");
  return type;
}


/*@} End of Static functions */

/*@{ Private functions */
void HLNodePrivate_setData(HL_Node* node, size_t datasize, unsigned char* data)
{
  HL_ASSERT((node != NULL), "node was NULL");
  HLHDF_FREE(node->data);
  node->data = data;
  node->dSize = datasize;
}

void HLNodePrivate_setRawdata(HL_Node* node, size_t datasize, unsigned char* data)
{
  HL_ASSERT((node != NULL), "node was NULL");
  HLHDF_FREE(node->rawdata);
  node->rawdata = data;
  node->rdSize = datasize;
}

int HLNodePrivate_setTypeIdAndDeriveFormat(HL_Node* node, hid_t type)
{
  hid_t tcopy = -1;
  HL_FormatSpecifier format = HLHDF_UNDEFINED;
  HL_ASSERT((node != NULL), "node was NULL");
  tcopy = H5Tcopy(type);
  format = HL_getFormatSpecifierFromType(type);

  if (tcopy < 0 || format == HLHDF_UNDEFINED) {
    HL_ERROR0("Could not set type and derive format");
    goto fail;
  }

  HL_H5T_CLOSE(node->typeId)
  node->typeId = tcopy;
  node->format = format;
  return 1;
fail:
  HL_H5T_CLOSE(tcopy);
  return 0;
}

char* HLNodePrivate_getName(HL_Node* node)
{
  HL_ASSERT((node != NULL), "node was NULL");
  return node->name;
}

void HLNodePrivate_setHdfID(HL_Node* node, hid_t hdfid)
{
  HL_ASSERT((node != NULL), "HLNodePrivate_setHdfID called with node == NULL");

  switch(node->type) {
  case ATTRIBUTE_ID: {
    HL_H5A_CLOSE(node->hdfId);
    node->hdfId = hdfid;
    break;
  }
  case DATASET_ID:
  case GROUP_ID:
  case TYPE_ID: {
    HL_H5O_CLOSE(node->hdfId);
    node->hdfId = hdfid;
    break;
  }
  case REFERENCE_ID:
    break;
  default: {
    if (node->hdfId >= 0) {
      HL_ERROR1("Strange node type, can't close it (%ld)",(long)node->hdfId);
    }
    break;
  }
  }
}

hid_t HLNodePrivate_getHdfID(HL_Node* node)
{
  HL_ASSERT((node != NULL), "HLNodePrivate_getHdfID called with node == NULL");
  return node->hdfId;
}

const hsize_t* HLNodePrivate_getDims(HL_Node* node)
{
  HL_ASSERT((node != NULL), "HLNodePrivate_getDims called with node == NULL");
  return (const hsize_t*)node->dims;
}

hid_t HLNodePrivate_getTypeId(HL_Node* node)
{
  HL_ASSERT((node != NULL), "HLNodePrivate_getDims called with node == NULL");
  return node->typeId;
}
/*@} End of Private functions */

/*@{ Interface functions */
HL_Node* newHL_Node(const char* name)
{
  HL_Node* retv = NULL;
  HL_SPEWDEBUG0("ENTER: newHL_Node");
  if (!name) {
    HL_ERROR0("When creating a nodelist item, name has to be specified");
    goto fail;
  }

  if (!(retv = (HL_Node*) malloc(sizeof(HL_Node)))) {
    HL_ERROR0("Failed to allocate HL_Node");
    goto fail;
  }
  retv->type = UNDEFINED_ID;
  retv->format = HLHDF_UNDEFINED;
  retv->name = strdup(name);
  retv->ndims = 0;
  retv->dims = NULL;
  retv->data = NULL;
  retv->rawdata = NULL;
  retv->typeId = -1;
  retv->dSize = 0;
  retv->rdSize = 0;
  retv->dataType = DTYPE_UNDEFINED_ID;
  retv->hdfId = -1;
  retv->mark = NMARK_CREATED;
  retv->compoundDescription = NULL;
  retv->compression = NULL;

  if (retv->name == NULL) {
    HL_ERROR0("Could not allocate memory when creating node");
    freeHL_Node(retv);
    retv = NULL;
  }
fail:
  return retv;
}

void freeHL_Node(HL_Node* node)
{
  HL_SPEWDEBUG0("ENTER: freeHL_Node");
  if (!node)
    return;

  if (node->typeId >= 0) {
    disableErrorReporting();
    H5Tclose(node->typeId);
    enableErrorReporting();
  }

  HLNodePrivate_setHdfID(node, -1);

  HLHDF_FREE(node->name);
  HLHDF_FREE(node->dims);
  HLHDF_FREE(node->data);
  HLHDF_FREE(node->rawdata);
  freeHL_CompoundTypeDescription(node->compoundDescription);
  freeHL_Compression(node->compression);
  HLHDF_FREE(node);
}

HL_Node* newHL_Group(const char* name)
{
  return newHL_NodeWithType(name, GROUP_ID);
}

HL_Node* newHL_Attribute(const char* name)
{
  return newHL_NodeWithType(name, ATTRIBUTE_ID);
}

HL_Node* newHL_Dataset(const char* name)
{
  return newHL_NodeWithType(name, DATASET_ID);
}

HL_Node* newHL_Datatype(const char* name)
{
  return newHL_NodeWithType(name, TYPE_ID);
}

HL_Node* newHL_Reference(const char* name)
{
  return newHL_NodeWithType(name, REFERENCE_ID);
}

HL_Node* copyHL_Node(HL_Node* node)
{
  hsize_t npts;
  HL_Node* retv = NULL;
  HL_SPEWDEBUG0("ENTER: copyHL_Node");
  if (!node)
    return NULL;

  retv = newHL_Node(node->name);
  if (retv == NULL) {
    goto fail;
  }
  if (!setHL_NodeDimensions(retv, node->ndims, node->dims)) {
    goto fail;
  }
  retv->type = node->type;
  retv->dSize = node->dSize;
  retv->rdSize = node->rdSize;
  if(!setHL_NodeDimensions(retv, node->ndims, node->dims)) {
    goto fail;
  }
  npts = getHL_NodeNumberOfPoints(retv);

  retv->data = (unsigned char*)malloc(npts*retv->dSize);
  memcpy(retv->data,node->data,npts*retv->dSize);

  if(node->rawdata!=NULL) {
    retv->rawdata = (unsigned char*)malloc(npts*retv->rdSize);
    memcpy(retv->rawdata,node->rawdata,npts*retv->rdSize);
  } else {
    retv->rdSize = 0;
    retv->rawdata = NULL;
  }
  retv->format = node->format;

  if(node->typeId>=0)
    retv->typeId=H5Tcopy(node->typeId);

  retv->dataType=node->dataType;
  //fprintf(stderr, "Copying node with hdfId = %d\n",node->hdfId);
  retv->hdfId=-1; //node->hdfId;
  retv->mark=node->mark;

  retv->compoundDescription=copyHL_CompoundTypeDescription(node->compoundDescription);

fail:
  return retv;
}

int setHL_NodeScalarValue(HL_Node* node, size_t sz, unsigned char* value,
  const char* fmt, hid_t typid)
{
  unsigned char* data = NULL;
  hid_t tmptypeid = -1;
  HL_FormatSpecifier format = HLHDF_UNDEFINED;
  int status = 0;

  HL_ASSERT((node != NULL), "setHL_NodeScalarValue called with node == NULL");
  HL_SPEWDEBUG0("ENTER: setHL_NodeScalarValue");

  format = HL_getFormatSpecifier(fmt);
  if (format == HLHDF_UNDEFINED || format == HLHDF_ARRAY) {
    HL_ERROR0("When setting a node value, fmt has to be reckognized");
    goto fail;
  }

  if ((data = (unsigned char*) malloc(sz))==NULL) {
    HL_ERROR0("Failed to allocate memory");
    goto fail;
  }
  memcpy(data, value, sz);

  if (format == HLHDF_STRING && typid < 0) {
    tmptypeid = HLNode_createStringType(sz);
    if (tmptypeid < 0) {
      HL_ERROR0("Failed to create string type\n");
      goto fail;
    }
  } else if (format == HLHDF_COMPOUND && typid < 0) {
    HL_ERROR0("Atempting to set compound data with no type id");
    goto fail;
  } else {
    if (typid < 0) {
      tmptypeid = HL_translateFormatStringToDatatype(fmt);
    } else {
      tmptypeid = H5Tcopy(typid);
    }
  }

  HLHDF_FREE(node->data);
  HL_H5T_CLOSE(node->typeId);
  node->data = data;
  node->format = format;
  node->dSize = sz;
  node->typeId = tmptypeid;
  data = NULL;
  tmptypeid = -1;
  node->dataType = HL_SIMPLE;
  if (node->mark != NMARK_CREATED)
    node->mark = NMARK_CHANGED;

  status = 1;
fail:
  HLHDF_FREE(data);
  HL_H5T_CLOSE(tmptypeid);
  return status;
}

int setHL_NodeArrayValue(HL_Node* node, size_t sz, int ndims, hsize_t* dims,
  unsigned char* value, const char* fmt, hid_t typid)
{
  int i;
  size_t npts = 0;
  unsigned char* data = NULL;
  HL_FormatSpecifier format = HLHDF_UNDEFINED;
  hid_t tmptypeid = -1;
  int status = 0;

  HL_ASSERT((node != NULL), "setHL_NodeArrayValue called with node == NULL");
  HL_ASSERT((ndims>0 && dims!=NULL), "setHL_NodeArrayValue called with inconsistant ndims and dims");

  HL_SPEWDEBUG0("ENTER: setHL_NodeArrayValue");

  format = HL_getFormatSpecifier(fmt);
  if (format == HLHDF_UNDEFINED || format == HLHDF_ARRAY) {
    HL_ERROR0("When setting a node value, fmt has to be reckognized");
    goto fail;
  }

  npts = 1;
  for (i = 0; i < ndims; i++) {
    npts *= dims[i];
  }

  if ((data = (unsigned char*) malloc(npts * sz)) == NULL) {
    HL_ERROR0("Failed to allocate memory when setting value");
    goto fail;
  }
  memcpy(data, value, npts * sz);

  if (format == HLHDF_STRING && typid < 0) {
    tmptypeid = HLNode_createStringType(sz);
    if (tmptypeid < 0) {
      HL_ERROR0("Failed to create string type\n");
      goto fail;
    }
  } else if (format == HLHDF_COMPOUND && typid < 0) {
    HL_ERROR0("Atempting to set compound data with no type id");
    goto fail;
  } else {
    if (typid < 0) {
      tmptypeid = HL_translateFormatStringToDatatype(fmt);
    } else {
      tmptypeid = H5Tcopy(typid);
    }
  }

  if (!setHL_NodeDimensions(node, ndims, dims)) {
    HL_ERROR0("Failed to set dimensions");
    goto fail;
  }

  HLHDF_FREE(node->data);
  HL_H5T_CLOSE(node->typeId);
  node->data = data;
  node->format = format;
  node->dSize = sz;
  node->typeId = tmptypeid;
  data = NULL;
  tmptypeid = -1;

  node->dataType = HL_ARRAY;

  if (node->mark != NMARK_CREATED)
    node->mark = NMARK_CHANGED;

  status = 1;
fail:
  HLHDF_FREE(data);
  HL_H5T_CLOSE(tmptypeid);
  return status;
}

char* getHL_NodeName(HL_Node* node)
{
  char* result = NULL;

  HL_ASSERT((node != NULL), "equalsHL_NodeName called with node == NULL");

  result = strdup(node->name);
  if (result == NULL) {
    HL_ERROR0("Failed to allocate memory");
  }
  return result;
}

const unsigned char* getHL_NodeData(HL_Node* node)
{
  HL_ASSERT((node != NULL), "getHL_NodeData called with node == NULL");
  return (const unsigned char*)node->data;
}

size_t getHL_NodeDataSize(HL_Node* node)
{
  HL_ASSERT((node != NULL), "getHL_NodeDataSize called with node == NULL");
  return node->dSize;
}

const unsigned char* getHL_NodeRawdata(HL_Node* node)
{
  HL_ASSERT((node != NULL), "getHL_NodeData called with node == NULL");
  return (const unsigned char*)node->rawdata;
}

size_t getHL_NodeRawdataSize(HL_Node* node)
{
  HL_ASSERT((node != NULL), "getHL_NodeRawdataSize called with node == NULL");
  return node->rdSize;
}

int equalsHL_NodeName(HL_Node* node, const char* name)
{
  HL_ASSERT((node != NULL), "equalsHL_NodeName called with node == NULL");

  if (name == NULL) {
    return 0;
  }

  return (strcmp(node->name, name) == 0 ? 1 : 0);
}

void setHL_NodeMark(HL_Node* node, const HL_NodeMark mark)
{
  HL_ASSERT((node != NULL), "setHL_NodeMark called with node == NULL");
  node->mark = mark;
}

HL_NodeMark getHL_NodeMark(HL_Node* node)
{
  HL_ASSERT((node != NULL), "getHL_NodeMark called with node == NULL");
  return node->mark;
}

void setHL_NodeType(HL_Node* node, const HL_Type type)
{
  HL_ASSERT((node != NULL), "setHL_NodeType called with node == NULL");
  node->type = type;
}

HL_Type getHL_NodeType(HL_Node* node)
{
  HL_ASSERT((node != NULL), "getHL_NodeType called with node == NULL");
  return node->type;
}

const char* HLNode_getFormatName(HL_Node* node)
{
  HL_ASSERT((node != NULL), "HLNode_getFormatName called with node == NULL");
  return HL_getFormatSpecifierString(node->format);
}

HL_FormatSpecifier HLNode_getFormat(HL_Node* node)
{
  HL_ASSERT((node != NULL), "HLNode_getFormat called with node == NULL");
  return node->format;
}

void setHL_NodeDataType(HL_Node* node, HL_DataType datatype)
{
  HL_ASSERT((node != NULL), "setHL_NodeDataType called with node == NULL");
  node->dataType = datatype;
}

HL_DataType getHL_NodeDataType(HL_Node* node)
{
  HL_ASSERT((node != NULL), "getHL_NodeDataType called with node == NULL");
  return node->dataType;
}


int setHL_NodeDimensions(HL_Node* node, int ndims, hsize_t* dims)
{
  hsize_t* tmpdims = NULL;
  HL_ASSERT((node != NULL), "setHL_NodeDimensions called with node == NULL");
  int status = 0;

  if (ndims > 0 && dims != NULL) {
    tmpdims = (hsize_t*)malloc(sizeof(hsize_t)*ndims);
    if (tmpdims != NULL) {
      memcpy(tmpdims, dims, sizeof(hsize_t)*ndims);
    } else {
      HL_ERROR0("Failed to allocate memory for dimensions");
      goto fail;
    }
  }

  HLHDF_FREE(node->dims);
  node->dims = tmpdims;
  node->ndims = ndims;
  tmpdims = NULL;
  status = 1;
fail:
  HLHDF_FREE(tmpdims);
  return status;
}

void getHL_NodeDimensions(HL_Node* node, int* ndims, hsize_t** dims)
{
  HL_ASSERT((node != NULL), "setHL_NodeDimensions called with node == NULL");

  if (ndims != NULL && dims != NULL) {
    *ndims = 0;
    *dims = NULL;
    if (node->ndims > 0 && node->dims != NULL) {
      *dims = (hsize_t*)malloc(sizeof(hsize_t)*node->ndims);
      if (*dims != NULL) {
        memcpy(*dims, node->dims, sizeof(hsize_t)*node->ndims);
        *ndims = node->ndims;
      } else {
        HL_ERROR0("Failed to allocate memory");
      }
    }
  } else {
    HL_ERROR0("Inparameters NULL");
  }
}

int getHL_NodeRank(HL_Node* node)
{
  HL_ASSERT((node != NULL), "getHL_NodeRank called with node == NULL");
  return node->ndims;
}

hsize_t getHL_NodeDimension(HL_Node* node, int index)
{
  HL_ASSERT((node != NULL), "getHL_NodeRank called with node == NULL");
  if (index < 0 || index >= node->ndims || node->dims == NULL) {
    return 0;
  }
  return node->dims[index];
}

hsize_t getHL_NodeNumberOfPoints(HL_Node* node)
{
  HL_ASSERT((node != NULL), "getHL_NodeRank called with node == NULL");
  if (node->ndims == 0) {
    return 1;
  }
  if (node->ndims > 0 && node->dims != NULL) {
    hsize_t n = 1;
    int i = 0;
    for (i = 0; i < node->ndims; i++) {
      n *= node->dims[i];
    }
    return n;
  }
  return 0;
}

void setHL_NodeCompoundDescription(HL_Node* node, HL_CompoundTypeDescription* descr)
{
  HL_ASSERT((node != NULL), "setHL_NodeCompoundDescription called with node == NULL");
  freeHL_CompoundTypeDescription(node->compoundDescription);
  node->compoundDescription = descr;
}

HL_CompoundTypeDescription* getHL_NodeCompoundDescription(HL_Node* node)
{
  HL_ASSERT((node != NULL), "getHL_NodeCompoundDescription called with node == NULL");
  return node->compoundDescription;
}

HL_Compression* getHL_NodeCompression(HL_Node* node)
{
  HL_ASSERT((node != NULL), "getHL_NodeCompression called with node == NULL");
  return node->compression;
}

void setHL_NodeCompression(HL_Node* node, HL_Compression* compression)
{
  HL_ASSERT((node != NULL), "setHL_NodeCompression called with node == NULL");
  freeHL_Compression(node->compression);
  node->compression = compression;
}

int commitHL_Datatype(HL_Node* node, hid_t testStruct_hid)
{
  HL_ASSERT((node != NULL), "commitHL_Datatype called with node == NULL");
  HL_SPEWDEBUG0("ENTER: commitDatatype");
  node->hdfId = testStruct_hid;
  HL_SPEWDEBUG0("EXIT: commitDatatype");
  return 1;
}
