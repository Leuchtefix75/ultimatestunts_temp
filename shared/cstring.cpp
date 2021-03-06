/***************************************************************************
                          cstring.cpp  -  bones very own easy string class ;-)
                             -------------------
    begin                : wo jan 15 2003
    copyright            : (C) 2003 by CJP
    email                : cornware-cjp@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <cstdio>
#include <cstdlib>
#include <cstdarg>

#include "cstring.h"

 using namespace std;

/*
	Some code comes from cstring.h, originally written by bones
*/

char CString::c_up(const char c) const
{
	if ((int (c) >= 97) && (int (c) <= 122)) {
		return ((char) (  ( (int) c) - 32));
	}
	return (c);
}

char CString::c_down(const char c) const
{
	if ((int (c) >= 65) && (int (c) <= 90)) {
		return ((char) (  ( (int) c) + 32));
	}
	return (c);
}

CString::CString(const char *s)
{
	this->assign(s);
}

CString::CString()
{
	this->assign("");
}

CString::CString(const CVector &v)
{
	this->assign("");
	operator= (CString(v.x) + "," + v.y + "," + v.z);
}

CString::CString(bool val)
{
	if(val)
		{this->assign("true");}
	else
		{this->assign("false");}
}

//
// LTrim: Trim left
//
CString & CString::trimL()
{
  while (this->length() > 0)
  {
    bool e = false;
    switch ((char) (*this)[0]) {
      case '\n':
      case (char) 1:
      case '\r':
      case ' ': this->erase(0,1);
                break;
      default: e = true;

    }
    if (e) break;
  }
  return (*this);
}

//
// RTrim: Trim right
//
CString & CString::trimR()
{
  while (this->length() > 0)
  {
    bool e = false;
    switch ((char) (*this)[this->length()-1]) {
      case '\n':
      case (char) 1:
      case '\r':
      case ' ': this->erase(this->length()-1,1);
                break;
      default: e = true;

    }
    if (e) break;
  }
  return (*this);
}

//
//  Trim: Trim string
//
CString & CString::Trim()
{
   trimR();
   trimL();
   return (*this);
}

CString & CString::toUpper()
{
  for (int unsigned i=0;i<this->size();i++) {
    (*this)[i] = c_up((*this)[i]);
  }
  return *this;
}

CString & CString::toLower()
{
  for (int unsigned i=0;i<this->size();i++) {
    (*this)[i] = c_down((*this)[i]);
  }
  return *this;
}

CString & CString::operator= (const char *str)
{
   this->assign(str);
   return *this;
}

CString CString::getFirstLine()
{
  CString *res = new CString();
  for (int unsigned i=0; i < this->length(); i++) {
     if (((*this)[i] == '\n') || ((*this)[i] == '\r')) {
//           for (;(((*this)[i+1] == '\n') || ((*this)[i+1] == '\r'));i++);
        if (i<this->length()-1) { if (((*this)[i+1] == '\n') || ((*this)[i+1] == '\r')) i++; }

        res->assign(this->subStr(0,i+1));
//           res->Trim();
        (*this) = this->subStr(i+1);
//           this->LTrim();
        return (*res);
     }
  }
  return ("");
}

int CString::countChar(const char c) const
{
  int res = 0;
  for (int unsigned i=0;i<this->length(); i++) {
    if ((*this)[i] == c) res++;
  }
  return  (res);
}

int CString::toInt()
{
  return (atol(this->c_str()));
}

CString & CString::dump() const
{
  CString *res = new CString();
  for (int unsigned i=0;i<this->length();i++) {
    char c=(*this)[i];
    char s[10];
    if ( ((int (c) >= 97) && (int (c) <= 122)) ||
         ((int (c) >= 65) && (int (c) <= 90))
       )
    {
       sprintf(s, "('%c' 0x%x)", c, (int) c);
    } else {
       sprintf(s, "('?' 0x%x)", (int) c);
    }
    *res+=s;
  }
  return (*res);
}

//CJP functions:
CString::CString(int val)
{
	char c[80];
	sprintf(c, "%d", val);
	assign(c);
}

CString::CString(unsigned int val)
{
	char c[80];
	sprintf(c, "%d", val);
	assign(c);
}

CString::CString(float val)
{
	char c[80];
	sprintf(c, "%f", val);
	assign(c);
}

CString &CString::fromTime(float t)
{
	int minutes = int(t / 60);
	t -= 60*minutes;
	int seconds = int(t);
	t -= seconds;
	int hundreds = int(100*t);

	CString secS = CString(seconds);
	CString hunS = CString(hundreds);
	if(secS.length() == 1) secS = CString("0") + secS;
	if(hunS.length() == 1) hunS = CString("0") + hunS;

	(*this) = secS + "." + hunS;

	if(minutes > 0)
		(*this) = CString(minutes) + ":" + (*this);

	return *this;
}

CString CString::operator+(CString const &val) const
{
	CString ret(*this);
	ret.append(val);
	return ret;
}

CString CString::operator+(const char *val) const
{
	CString ret(*this);
	ret.append(val);
	return ret;
}

CString operator+(const char *val1, CString const &val2)
{
	CString ret(val1);
	ret.append(val2);
	return ret;
}

bool CString::operator!= (const char *str) const
{
	return !(*this == str);
}

bool CString::operator!= (const CString &val) const
{
	return !(*this == val);
}

CString CString::mid(unsigned int i, int l) const
{
	if(i > length())
		return "";
	if (l == -1)
		{ l = length() - i;}

	CString ret;
	ret.assign(*this, i, l);
	return ret;

	/*
	char *c = new char[l+1];
	strncpy(c, c_str()+i, l);
	*(c+l) = '\0';

	CString ret;
	ret = c;
	delete [] c;
	return ret;
	*/
}

CString CString::right(unsigned int len) const
{
	if(len >= length()) return *this;

	CString ret;
	ret.assign(*this, length()-len, len);
	return ret;
}

//actually the same as mid:
CString & CString::subStr(const int unsigned pos, const int n) const
{
 CString *res = new CString("");
 if (pos <= this->length()) {
    int len = n;
    if (len == -1) { len = this->size() -pos; }
    res->assign(*this, pos, len);
 }
 return (*res);
}

int CString::inStr(char c) const
{
	/*
	int ret = -1;
	for(unsigned int i=0; i<length(); i++)
		if ((*this)[i] == c)
			{ret = i; break;}

	return ret;
	*/
    return find(c);
}

int CString::inStr(const CString &s) const
{
	return find(s);
}

CString &CString::format(const char *format, int maxsize, ...)
{
	char buffer[maxsize];

	va_list ap;
	va_start(ap, maxsize);
	vsnprintf(buffer, maxsize, format, ap);
	va_end(ap);

	assign(buffer);
	return *this;
}

float CString::toFloat()
{
    float ret;
    if(sscanf(c_str(), " %f ", &ret) < 1) //watch the spaces
    	return 0.0;

    return ret;
}

CVector CString::toVector()
{
	float x = toFloat();

	int comma = inStr(',');
	if(comma < 0)
		return CVector(x, 0.0, 0.0);
	CString sub = mid(comma+1);

	float y = sub.toFloat();

	comma = sub.inStr(',');
	if(comma < 0)
		return CVector(x, y, 0.0);

	sub = sub.mid(comma+1);
	float z = sub.toFloat();
	return CVector(x,y,z);
}

CString CString::forceLength(unsigned int l) const
{
	if(l <= length())
		return mid(0, l);

	CString ret = *this;
	for(unsigned int i=0; i<l-length(); i++)
		ret += ' ';

	return ret;
}
