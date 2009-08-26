#pragma once
/*
 *  TargetPhrase.h
 *  CreateBerkeleyPt
 *
 *  Created by Hieu Hoang on 31/07/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include <vector>
#include "Phrase.h"
#include "../../moses/src/TypeDef.h"
#include "../../moses/src/LMList.h"
#include "../../moses/src/TargetPhrase.h"

class Db;

namespace Moses
{
	class PhraseDictionary;
}

namespace MosesBerkeleyPt
{

class TargetPhrase : public Phrase
{
protected:
	typedef std::pair<int, int>  AlignPair;
	typedef std::vector<AlignPair> AlignType;
	AlignType m_align;
	std::vector<float>	m_scores;
	std::vector<Word>		m_headWords;
	Moses::UINT32 m_targetId; // set when saved to db

	Moses::CountInfo m_countInfo;

	char *WritePhraseToMemory(size_t &memUsed,  int numScores, size_t sourceWordSize, size_t targetWordSize) const;

	size_t WriteAlignToMemory(char *mem) const;
	size_t WriteScoresToMemory(char *mem) const;
	size_t WriteCountsToMemory(char *mem) const;

	size_t ReadAlignFromMemory(const char *mem);
	size_t ReadScoresFromMemory(const char *mem, size_t numScores);
	size_t ReadCountsFromMemory(const char *mem);

	size_t ReadPhraseFromMemory(const char *mem, size_t numFactors);

public:
	TargetPhrase();
	virtual ~TargetPhrase();
	
	void CreateAlignFromString(const std::string &alignString);
	void CreateScoresFromString(const std::string &inString, size_t numScores);
	void CreateHeadwordsFromString(const std::string &inString, Vocab &vocab);

	const Word &GetHeadWords(size_t ind) const
	{ return m_headWords[ind]; }
	
	const AlignType &GetAlign() const
	{ return m_align; }
	size_t GetAlign(size_t sourcePos) const;
	
	const std::vector<float> &GetScores() const
	{ return m_scores; }

	Moses::UINT32 SaveTargetPhrase(Db &dbTarget, Db &dbTargetInd, Moses::UINT32 &nextTargetId
												,int numScores, size_t sourceWordSize, size_t targetWordSize);	

	size_t ReadOtherInfoFromMemory(const char *mem
																, size_t numSourceFactors, size_t numTargetFactors
																, size_t numScores);
	char *WriteOtherInfoToMemory(size_t &memUsed, int numScores, size_t sourceWordSize, size_t targetWordSize) const;

	void Load(Db &db, size_t numTargetFactors);
	
	Moses::TargetPhrase *ConvertToMoses(const std::vector<Moses::FactorType> &inputFactors
																			, const std::vector<Moses::FactorType> &outputFactors
																			, const Vocab &vocab
																			, const Moses::PhraseDictionary &phraseDict
																			, const std::vector<float> &weightT
																			, float weightWP
																			, const Moses::LMList &lmList
																			, const Moses::Phrase &sourcePhrase) const;
	
	const  Moses::CountInfo &GetCountInfo() const
	{ return m_countInfo; }
	void CreateCountInfo(const std::string &countStr);
};
	
} // namespace

