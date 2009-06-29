/**
 * Functions for working with HL_NodeList's.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-06-24
 */
#ifndef HLHDF_NODELIST_H
#define HLHDF_NODELIST_H
#include "hlhdf_types.h"

/**
 * Creates a new HL_NodeList instance
 * @ingroup hlhdf_c_apis
 * @return the allocated node list on success, otherwise NULL.
 */
HL_NodeList* newHL_NodeList(void);

/**
 * Releasing all resources associated with this node list including the node list itself.
 * @ingroup hlhdf_c_apis
 * @param[in] nodelist the list that should be released.
 */
void freeHL_NodeList(HL_NodeList* nodelist);

/**
 * Sets the filename in the HL_NodeList instance
 * @param[in] nodelist - the nodelist
 * @param[in] filename - the filename that should be used
 * @return 1 on success, otherwise 0
 */
int setHL_NodeListFileName(HL_NodeList* nodelist, const char* filename);

/**
 * Returns the filename of this nodelist.
 * @param[in] nodelist - the nodelist
 * @return the filename for this nodelist or NULL if no filename is set or failed to allocate memory.
 */
char* getHL_NodeListFileName(HL_NodeList* nodelist);

/**
 * Returns the number of nodes that exists in the provided nodelist.
 * @param[in] nodelist - the node list
 * @return the number of nodes or a negative value on failure.
 */
int getHL_NodeListNumberOfNodes(HL_NodeList* nodelist);

/**
 * Returns the node at the specified index.
 * @param[in] nodelist - the node list
 * @param[in] index - the index of the node
 * @return the node if it exists, otherwise NULL. <b>Do not free since it is an internal pointer</b>
 */
HL_Node* getHL_NodeListNodeByIndex(HL_NodeList* nodelist, int index);

/**
 * Marks all nodes in the nodelist with the provided mark.
 * @param[in] nodelist - the nodelist to be updated.
 * @param[in] mark - the mark each node should have.
 */
void markHL_NodeListNodes(HL_NodeList* nodelist, const HL_NodeMark mark);

/**
 * Adds a node to the nodelist.
 * @ingroup hlhdf_c_apis
 * @param[in] nodelist the nodelist that should get a node added
 * @param[in] node the node that should be added to the node list
 * @return 1 On success, otherwise 0
 */
int addHL_Node(HL_NodeList* nodelist, HL_Node* node);

/**
 * Locates a node called nodeName in the nodelist and returns a pointer
 * to this node. I.e. Do not delete it!
 * @ingroup hlhdf_c_apis
 * @param[in] nodelist the nodelist that should be searched in
 * @param[in] nodeName the name of the node that should be located
 * @return the node if it could be found, otherwise NULL.
 */
HL_Node* getHL_Node(HL_NodeList* nodelist,const char* nodeName);

/**
 * Searches the nodelist for any type node, that has got the same object id as objno0 and objno1.
 * @ingroup hlhdf_c_apis
 * @param[in] nodelist the nodelist that should be searched
 * @param[in] objno0 identifier 0
 * @param[in] objno1 identifier 1
 * @return The compound type description if found, otherwise NULL.
 */
HL_CompoundTypeDescription* findHL_CompoundTypeDescription(HL_NodeList* nodelist,
                  unsigned long objno0,
                  unsigned long objno1);

#endif /* HLHDF_NODELIST_H */