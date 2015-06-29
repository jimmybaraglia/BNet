/* 
 * Copyright (C): None
 * Authors: Jimmy Baraglia
 * Public License for more details
*/

#include "BNet.h"

BNet::BNet(int argc, char * argv[]){
}

void BNet::initProbaMatrix(){
	
	nodesProbability = new MatrixXf(listOfNodes[0]->getNumberOfStates(), listOfNodes.size());
	
	for(int i = 0; i < listOfNodes[0]->getNumberOfStates(); i++){
		for(int j = 0; j < listOfNodes.size(); j++){
			nodesProbability[0](i,j) = -1;
		}  
	}
}

bool BNet::run(){
	initProbaMatrix();
	
	for(int i = 0; i < listOfNodes.size(); i++){ //!! change back to 0
		setProbaNode(listOfNodes[i]);
	}
	return true;
}

void BNet::setProbaNode(BNode* i_node){
	//If the state is already known (as an evidence) the proability is set
	//If not, the probability of the parent is looked at
	
		
	if(i_node->getState() != -1){
		for(int row = 0; row < nodesProbability->rows(); row++){
			if(row == i_node->getState()){
				nodesProbability[0](row, getIdOfNode(i_node)) = 1; 
			}else{
				nodesProbability[0](row, getIdOfNode(i_node)) = 0; 
			}
		}
	}else{
		for(int i = 0; i < listOfNodeInputEdges[getIdOfNode(i_node)]->rows(); i ++){
			//If any of the value in the parents' probability matrice is -1, it means that the probability has't yet been calculated
			if((listOfNodeInputEdges[getIdOfNode(i_node)][0](i) != getIdOfNode(i_node)	) && nodesProbability[0](0,listOfNodeInputEdges[getIdOfNode(i_node)][0](i)) == -1){
				setProbaNode(listOfNodes[listOfNodeInputEdges[getIdOfNode(i_node)][0](i)]);
			}
		}
		//Calculate own proba based on parents' proba
		//!! This part only works for binary nodes!!//
		for(int j = 0; j < i_node->getNumberOfStates(); j ++){
			double proba = 0;
			BPVector parentsState;
			for(int k = 0; k < listOfNodeProbabilities[getIdOfNode(i_node)]->cols(); k++){
				double proba2 = 1;
				double proba1 = (double)listOfNodeProbabilities[getIdOfNode(i_node)][0](j, k);
				
				int mult = ((pow(2, getNumberOfParentsOf(i_node))))/2;
				for(int i = 0; i < listOfNodeInputEdges[getIdOfNode(i_node)]->rows(); i ++){
					if((listOfNodeInputEdges[getIdOfNode(i_node)][0](i) != getIdOfNode(i_node))){
						int state = 1;
						if(k < mult)
							state = 0;
						
						proba2 = nodesProbability[0](state, listOfNodeInputEdges[getIdOfNode(i_node)][0](i));
						
						mult/2;
					}
				}
				proba += proba1*proba2;
				
			}
			nodesProbability[0](j,getIdOfNode(i_node)) = proba;
		}
	}
}

/*addNode(BNode *node)
  * Add a new node to the bayesian network.
  * returns 1 is successful
  * returns 0 is a node was already added with the same name
  * retunrs -1 if unknown error occured
  */
int BNet::addNode(BNode* i_node){
	int returnValue = -1;
	
	//If the node is already included in listOfNodes, we return 0
	for(auto i: listOfNodes){
		if( i->getName() == i_node->getName() ){
			returnValue = 0;
			break;
		}
	}
	
	//If nde not included, we add it
	if(returnValue != 0){
		listOfNodes.push_back(i_node);
		
		MatrixXf *newProbabilityMatrix = new MatrixXf(i_node->getNumberOfStates(),1);
		listOfNodeProbabilities.push_back(newProbabilityMatrix);
		
		VectorXd *newInputEdgeVector = new VectorXd(1);
		newInputEdgeVector[0](0) = getIdOfNode(i_node);
		listOfNodeInputEdges.push_back(newInputEdgeVector);
		returnValue = 1;
	}
	
	initProbaMatrix();
	
	return returnValue;
}

/*addEdge(BEdge *edge)
  * Add a new edge to the bayesian network.
  * returns 1 is successful
  * returns 0 is a edge was already added
  * retunrs -1 if unknown error occured
  */
int BNet::addEdge(BEdge* i_edge){
	int returnValue = -1;
	
	//If the edge is already included in listOfEdges, we return 0
	for(auto i: listOfEdges){
		if( (i->getParent()->getName() == i_edge->getParent()->getName()) &&  
		    (i->getChild()->getName() == i_edge->getChild()->getName())){
			returnValue = 0;
			break;
		}
	}
	
	//If nde not included, we add it
	if(returnValue != 0){
		listOfEdges.push_back(i_edge);
 		updateNodeProbabilityMatrice(getIdOfNode(i_edge->getChild()), getIdOfNode(i_edge->getParent()));
		returnValue = 1;
	}
	return returnValue;
}

/*updateNodeProbabilityMatrices
  * Update the size of the matrices when a node or and edge is added/removed
  */
void BNet::updateNodeProbabilityMatrice(int i_nodeNumber, int i_newEdgeFromNodeID){
  
	MatrixXf oldMatrix(listOfNodeProbabilities[i_nodeNumber]->cols(), listOfNodeProbabilities[i_nodeNumber]->rows()); 
 	oldMatrix = listOfNodeProbabilities[i_nodeNumber][0];
	
	VectorXd oldVector(listOfNodeInputEdges[i_nodeNumber]->rows()); 
 	oldVector = listOfNodeInputEdges[i_nodeNumber][0];
	
	//Create a matrix with cols being the number of states of the node and rows all the possibilities for its parent value
	listOfNodeProbabilities[i_nodeNumber] = new MatrixXf(listOfNodes[i_nodeNumber]->getNumberOfStates(), pow(2, getNumberOfParentsOf(listOfNodes[i_nodeNumber])));
	
	for(int cRows = 0; cRows < listOfNodeProbabilities[i_nodeNumber]->rows(); cRows++){  
		for(int cCols = 0; cCols < listOfNodeProbabilities[i_nodeNumber]->cols(); cCols++){
			if(oldMatrix.cols() > cCols && oldMatrix.rows() > cRows){
				listOfNodeProbabilities[i_nodeNumber][0](cRows, cCols) = oldMatrix(cRows, cCols); 
			}
		}
	}
	
	//Create a matrix with rows being the number of states of the node and rows all the possibilities for its parent value
	listOfNodeInputEdges[i_nodeNumber] = new VectorXd(getNumberOfParentsOf(listOfNodes[i_nodeNumber]));
	
	for(int cRows = 0; cRows < listOfNodeInputEdges[i_nodeNumber]->rows(); cRows++){
		if(oldVector.rows() > cRows){
			listOfNodeInputEdges[i_nodeNumber][0](cRows) = oldVector(cRows); 
		}
	}
	listOfNodeInputEdges[i_nodeNumber][0]((listOfNodeInputEdges[i_nodeNumber]->rows())-1) = i_newEdgeFromNodeID;
  
}
    
    
/*setNodeProbability(BNode* i_node, BPVector i_parentsStates, int i_inputNodeState, int i_probability)
  * set the probability for a node to be a given state knowing the state of his parents
  * returns 1 is successful
  * retunrs -1 if unknown error occured
  * !! set nodes' probabilities everytime you add or remove an edge or node
  */
int BNet::setNodeProbability(BNode* i_node, BPVector* i_parentsStates, int i_inputNodeState, float i_probability){
	int idCol, idRow;
	int idNode;
	
	//The row number is equivalent to the state number
	idRow = i_inputNodeState;
	
	//Get the id of the node to find the correct probability matrix
	idNode = getIdOfNode(i_node);
	
	//If the number of parents in the BPVector is different from the real number of parents
	if(i_parentsStates->getSizeOfParentList() != getNumberOfParentsOf(i_node)){
		cerr<<"Wrong number of parent in BPVector in classs setNodeProbability"<<endl;
		return -1;
	}
	
	//If the size of the probability matrix is not what it should be
	if(listOfNodeProbabilities[idNode]->cols() != (pow(2, getNumberOfParentsOf(i_node)))){
		cerr<<"Error: probability matrix size mismatch in class setNodeProbability"<<endl;
		return -1;
	}
	
	
	idCol = getCol(i_node, i_parentsStates);	
	
	//For now the proba are saved as percentages
	listOfNodeProbabilities[getIdOfNode(i_node)][0](idRow, idCol) = (float)i_probability;
	
}

/*newEvidence(BNode* i_node, int i_state)
  * Add a state to a node
  * returns 1 is successful
  * retunrs -1 if unknown error occured
  */
int BNet::newEvidence(BNode* i_node, int i_state){
	int returnValue = 1;
	
	int idOfNode = getIdOfNode(i_node);
	
	if(idOfNode != -1){
		listOfNodes[idOfNode]->setState(i_state);
		initProbaMatrix();
	}else{
		returnValue = -1;
	}
	
	return returnValue;
}

//Get methods listed below


int BNet::getNumberOfNodes(){
	return this->listOfNodes.size();
}

int BNet::getNumberOfEdges(){
	return this->listOfEdges.size();
}

int BNet::getIdOfNode(BNode* i_node){
	int id = -1;
		
	for(int count = 0; count < listOfNodes.size(); count++){
		if(listOfNodes[count]->getName() == i_node->getName()){
			id = count;
			break;
		}
	}
	
	return id;
}
    
int BNet::getNumberOfParentsOf(BNode* i_node){
	int numberOfParents = 0;
		
	for(auto i: listOfEdges){
		if(i->getChild()->getName() == i_node->getName()){
			numberOfParents++;
		}
	}
	
	return numberOfParents;
}


BNode* BNet::getParentNb(BNode* i_node, int parentIndex){
	int count = 0;
		
	for(auto i: listOfNodes){
		if(i->getName() == i_node->getName()){
			if(count == parentIndex)
				return i;
			count++;
		}
	}
	
	return NULL;
}

    
float BNet::getNodeProbability(BNode* i_node, BPVector* i_parentsStates, int i_inputNodeState){
	int idCol, idRow;
	int idNode;
	
	//The row number is equivalent to the state number
	idRow = i_inputNodeState;
	
	//Get the id of the node to find the correct probability matrix
	idNode = getIdOfNode(i_node);
	
	//If the number of parents in the BPVector is different from the real number of parents
	if(i_parentsStates->getSizeOfParentList() != getNumberOfParentsOf(i_node)){
		cerr<<"Wrong number of parent in BPVector in classs getNodeProbability"<<endl;
		return -1;
	}
	
	//If the size of the probability matrix is not what it should be
	if(listOfNodeProbabilities[idNode]->cols() != (pow(2, getNumberOfParentsOf(i_node)))){
		cerr<<"Error: probability matrix size mismatch in class getNodeProbability"<<endl;
		return -1;
	}
	
	idCol = getCol(i_node, i_parentsStates);	
	
	return listOfNodeProbabilities[getIdOfNode(i_node)][0](idRow, idCol);
}

inline int BNet::getCol(BNode* i_node, BPVector* i_parentsStates){
	int idCol = 0;
	
	int mult = ((pow(2, getNumberOfParentsOf(i_node))))/2;
	for(auto i = 0; i < listOfNodeInputEdges[getIdOfNode(i_node)]->rows(); i++){
		idCol += mult*i_parentsStates->getStateOf(listOfNodes[listOfNodeInputEdges[getIdOfNode(i_node)][0](i)]);
		mult /= 2;
	}
	
	return idCol;
}

string BNet::toString(){
	string returnString = "";
  
	if(nodesProbability!= NULL && nodesProbability[0](0,0) != -1){
		for(auto cRows = 0; cRows < nodesProbability->rows(); cRows++){  
			for(auto cCols = 0; cCols < nodesProbability->cols(); cCols++){
				returnString += "P(" + listOfNodes[cCols]->getName() + "=" + to_string(cRows) + ") = " + to_string((float)nodesProbability[0](cRows, cCols)) + "; ";
			}
			returnString += "\n";
		}
	}else{
		returnString = "Probability matrix no yet runned \n";
	}
	
	return returnString;
}


BNet::~BNet(){
  
}


