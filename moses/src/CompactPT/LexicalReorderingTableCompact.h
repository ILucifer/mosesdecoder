// $Id$                                                                                                                               
// vim:tabstop=2                                                                                                                      
/***********************************************************************                                                              
Moses - factored phrase-based language decoder                                                                                        
Copyright (C) 2006 University of Edinburgh                                                                                            
                                                                                                                                      
This library is free software; you can redistribute it and/or                                                                         
modify it under the terms of the GNU Lesser General Public                                                                            
License as published by the Free Software Foundation; either                                                                          
version 2.1 of the License, or (at your option) any later version.                                                                    
                                                                                                                                      
This library is distributed in the hope that it will be useful,                                                                       
but WITHOUT ANY WARRANTY; without even the implied warranty of                                                                        
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU                                                                     
Lesser General Public License for more details.                                                                                       
                                                                                                                                      
You should have received a copy of the GNU Lesser General Public                                                                      
License along with this library; if not, write to the Free Software                                                                   
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA                                                        
***********************************************************************/  

#ifndef moses_LexicalReorderingTableCompact_h
#define moses_LexicalReorderingTableCompact_h

#include "LexicalReorderingTable.h"
#include "StaticData.h"
#include "PhraseDictionary.h"
#include "GenerationDictionary.h"
#include "TargetPhrase.h"
#include "TargetPhraseCollection.h"

#include "BlockHashIndex.h"
#include "CanonicalHuffman.h"
#include "StringVector.h"

namespace Moses {

class LexicalReorderingTableCompact: public LexicalReorderingTable
{
  private:
    bool m_inMemory;
    
    size_t m_numScoreComponent;
    bool m_multipleScoreTrees;
    
    BlockHashIndex m_hash;
    
    typedef CanonicalHuffman<float> ScoreTree;  
    std::vector<ScoreTree*> m_scoreTrees;
    
    StringVector<unsigned char, unsigned long, MmapAllocator>  m_scoresMapped;
    StringVector<unsigned char, unsigned long, std::allocator> m_scoresMemory;

    std::string MakeKey(const Phrase& f, const Phrase& e, const Phrase& c) const;
    std::string MakeKey(const std::string& f, const std::string& e, const std::string& c) const;
    
  public:
    LexicalReorderingTableCompact(
                                const std::string& filePath,
                                const std::vector<FactorType>& f_factors,
                                const std::vector<FactorType>& e_factors,
                                const std::vector<FactorType>& c_factors);
  
    LexicalReorderingTableCompact(
                                const std::vector<FactorType>& f_factors,
                                const std::vector<FactorType>& e_factors,
                                const std::vector<FactorType>& c_factors);
  
    virtual ~LexicalReorderingTableCompact();

    virtual std::vector<float> GetScore(const Phrase& f, const Phrase& e, const Phrase& c);
    void Load(std::string filePath);
};

}

#endif
