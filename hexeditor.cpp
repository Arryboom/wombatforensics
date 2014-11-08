/* $Id: hexEditor.cpp,v 1.14 2006-11-05 04:42:43 ganzhorn Exp $
 * This file is part of lfhex.
 * Copyright (C) 2006 Salem Ganzhorn <eyekode@yahoo.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/*
 *
 * Copyright (C) 2013 Pasquale Rinaldi <pjrinaldi@gmail.com>
 * Modifying the code to open tsk_img_byte data in addition to a real file.
 * Modifying to make it a viewer and not an editor.
 * Modifying to add a statusbar providing conversions.
 *
 */ 

#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <algorithm>
#include <iostream>

#include <QApplication>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QFocusEvent>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QMouseEvent>

#include "hexeditor.h"

extern int errno;

HexEditor::HexEditor( QWidget * parent, TskObject* tskobjptr )
    : QWidget(parent)
{
    tskptr = tskobjptr;
  _cols   = 8;
  _rows   = 10;
  _charsPerByte   = 2;
  _base           = 16;
  _topLeft        = 0;
  _topMargin = _wordSpacing    = 6;
  _bytesPerWord   = 2;
  _lastValidWord  = -1;
  _previousstep = 0;
  _selection[SelectionStart] = _selection[SelectionEnd] = -1;

  setFocusPolicy(Qt::StrongFocus);
  // the first setBytesPerWord should not come before the first setFont()
  QFont font("fixed");
  font.setStyleHint(QFont::TypeWriter);
  font.setFixedPitch(1);
  setFont( font );
}

HexEditor::~HexEditor() 
{
  _reader.close();
}

void HexEditor::ClearContent()
{
    //_reader.Clear();
    //_reader.seekimage(_topLeft);
    //_reader.seek(_topLeft);
    //_reader.readimage(_data,bytesPerPage());
    //setTopLeft(0);
}

void HexEditor::SetTopLeft(off_t offset)
{
    setTopLeft(offset);
}

QString HexEditor::filename() const
{
  return _reader.filename();
}

bool HexEditor::openimage()
{
    if(!_reader.openimage(tskptr))
        QMessageBox::critical(this, "HexEdit", "Error opening image\n", QMessageBox::Ok, 0);
    _cursor.setRange(0, _reader.size());
    _cursor.setCharsPerByte(_charsPerByte);
    setSelection(SelectionStart, -1);
    setSelection(SelectionEnd, -1);
    emit rangeChanged(0, _reader.size()/bytesPerLine());
    emit StepValues(1, bytesPerPage()/bytesPerLine());
    calculateFontMetrics();
    setTopLeft(0);

    return true;
}

bool HexEditor::open( const QString & filename )
{
  if(!_reader.open(C_STR(filename))) {
    QMessageBox::critical( this, "HexEdit", 
			   "Error loading \"" + filename + "\"\n" +
			  _reader.lastError(),
			   QMessageBox::Ok,0);
    return false;
  }

  // set the new range for the scrollbar
  _cursor.setRange(0,_reader.size());
  _cursor.setCharsPerByte(_charsPerByte);
  setSelection(SelectionStart,-1);
  setSelection(SelectionEnd,-1);
  emit rangeChanged(0,_reader.size()/bytesPerLine());
  emit StepValues(1, bytesPerPage()/bytesPerLine());
  calculateFontMetrics();     // update labels
  setTopLeft(0);              // reset the GUI
  return true;
}

void HexEditor::setBytesPerWord( int nbytes )
{
  _bytesPerWord = nbytes;
  calculateFontMetrics();
  QResizeEvent *re = new QResizeEvent(QSize(size()),QSize(size()));
  resizeEvent(re);
  delete re;
}
int HexEditor::fontHeight() const
{
    return _fontHeight;
}
int HexEditor::lineSpacing() const
{
  return _lineSpacing;
}
int HexEditor::fontMaxWidth() const
{
  return _fontMaxWidth;
}
void HexEditor::calculateFontMetrics()
{
  _lineSpacing = fontMetrics().lineSpacing();
  _fontMaxWidth = fontMetrics().maxWidth();
  _wordWidth   = _fontMaxWidth*charsPerWord();
  _fontHeight  = fontMetrics().height();
  // see how many bytes are needed to show the size of this file
  // log base 16 -> log16(x) = log(x)/log(16)
  if( _reader.size() ) {
     double width = log(static_cast<float>(_reader.size()))/log(16.0)+2;
     _offsetLabelBytes = static_cast<int>( width );
  }
  else
  {
     _offsetLabelBytes = 0;
  }
  _leftMargin = _topMargin + _fontMaxWidth*(_offsetLabelBytes + 2);
  // make sure bboxes are updated with new offset subdivision
  QResizeEvent *re = new QResizeEvent(QSize(size()),QSize(size()));
  resizeEvent(re);
  delete re;
}
void HexEditor::setFont(const QFont& font )
{ 
  if( !font.fixedPitch() ) {
    cerr << "setFont() failed, font was not fixedPitch()." << endl;
    return;
  }
  QWidget::setFont(font);
  calculateFontMetrics();
}
// set the top left editor to offset in reader
void HexEditor::setTopLeft( off_t offset )
{
  static bool inTopLeft;
  if( inTopLeft ) {
     // don't nest
     return;
  }
  inTopLeft = true;
  try {
     if( offset < 0 ) {
	_topLeft = 0;
     } else if( offset > _reader.size() ) {
	_topLeft = _reader.size();
     } else {
	_topLeft = offset;
     }
     // only let _topLeft be an integer multiple of the line length (round down)
     off_t linenum = _topLeft/bytesPerLine();
     //_topLeft = _topLeft*bytesPerLine();
     //_topLeft = (_topLeft/bytesPerLine()) * bytesPerLine();
     // update the labels
     //setOffsetLabels(_topLeft);
     _reader.seekimage(_topLeft);
     //_reader.seek(_topLeft);
     _reader.readimage(_data,bytesPerPage());
     //_reader.read(_data,bytesPerPage());
     
     repaint();
     emit topLeftChanged(linenum);
     //emit topLeftChanged(_topLeft);
  } catch( const exception &e ) {
     inTopLeft = false;
     throw e;
  }
  inTopLeft = false;
}

//void HexEditor::setOffsetLabels( off_t topLeft )
//{
  // need to impliment manually printing labels
//}

int HexEditor::topMargin() const
{
  return _topMargin;
}
int HexEditor::leftMargin() const
{
  return _leftMargin;
}
//
// access fn's for offset manip
//
int HexEditor::bytesPerPage() const
{
  return _rows*_cols*bytesPerWord();
}
int HexEditor::bytesPerWord() const
{
  return _bytesPerWord;
}
int HexEditor::bytesPerLine() const
{
  return bytesPerWord()*wordsPerLine();
}
int HexEditor::wordsPerLine() const
{
  return _cols;
}
int HexEditor::linesPerPage() const
{
  return _rows;
}
int HexEditor::wordsPerPage() const
{
  return _rows*_cols;
}
int HexEditor::charsPerByte() const
{
  return _charsPerByte;
}
int HexEditor::charsPerWord() const
{
  return _charsPerByte*bytesPerWord();
}
int HexEditor::charsPerLine() const
{
  return _charsPerByte*(bytesPerLine());
}
// translate local byte offsets to global byte offsets
off_t HexEditor::globalOffset( off_t local ) const
{
  return local+_topLeft;
}
// translate global byte offsets to viewport byte offsets
off_t HexEditor::localOffset( off_t global ) const
{
  return global-_topLeft;
}

int HexEditor::offsetToPercent(
   off_t offset
   )
{
   // round up
   return _reader.size() ? (int)ceil(100.0*offset/_reader.size()) : 0;
}

// public slots:

QRect HexEditor::charBBox( off_t charIdx ) const {
  int wordIdx = (charIdx/charsPerByte())/bytesPerWord();
  int localCharIdx = charIdx % charsPerWord();
  return QRect( _wordBBox[wordIdx].left() + localCharIdx*fontMaxWidth() +
		wordSpacing()/2, 
		_wordBBox[wordIdx].top(),
		fontMaxWidth(),
		fontHeight() );
}

QRect HexEditor::byteBBox( off_t byteIdx ) const {
  int wordIdx = byteIdx/bytesPerWord();
  int localByteIdx = byteIdx % bytesPerWord();
  return QRect( _wordBBox[wordIdx].left() + localByteIdx*2*fontMaxWidth() +
		wordSpacing()/2, 
		_wordBBox[wordIdx].top(),
		fontMaxWidth()*2,
		lineSpacing() );
}

QRect HexEditor::abyteBox(off_t byteIdx) const
{
    int wordIdx = byteIdx/bytesPerWord();
    int localByteIdx = byteIdx % bytesPerWord();
    return QRect(_asciiBBox[wordIdx].left() + localByteIdx*fontMaxWidth() + wordSpacing(), _asciiBBox[wordIdx].top(), fontMaxWidth(), lineSpacing());
}

void HexEditor::setTopLeftToPercent( int percent )
{
    percent = percent*bytesPerLine();
    if(_previousstep < percent)
    {
        int stepdiff = percent - _previousstep;
        if(stepdiff == bytesPerLine())
            nextLine();
        else if(stepdiff == bytesPerPage())
            nextPage();
        else
            setTopLeft(percent);
    }
    else if(_previousstep > percent)
    {
        int stepdiff = _previousstep - percent;
        if(stepdiff == bytesPerLine())
            prevLine();
        else if(stepdiff == bytesPerPage())
            prevPage();
        else
            setTopLeft(percent);
    }
    _previousstep = percent;
}

// 
// slot for setting cursor offset.
//
void HexEditor::setOffset( off_t offset )
{
  off_t oldWordOffset = localWordOffset();
  _cursor.setOffset( offset, 0 );
  // updateWord clamps the wordIdx to [0,_rows*_cols)
  updateWord( oldWordOffset ); 
  emit offsetChanged( _cursor.byteOffset() );
}

void HexEditor::nextLine()
{
  setTopLeft(_topLeft+bytesPerLine());
}
void HexEditor::prevLine()
{
  setTopLeft(_topLeft-bytesPerLine());
}
void HexEditor::nextPage()
{
  setTopLeft(_topLeft+bytesPerPage());
}
void HexEditor::prevPage()
{
  setTopLeft(_topLeft-bytesPerPage());
}

off_t HexEditor::localByteOffsetAtXY(off_t x, off_t y) 
{
  off_t wordIdx;
  off_t wordLength = wordSpacing()+wordWidth();
  off_t line = min(y/lineSpacing(),(off_t)linesPerPage());
  
  // constrain x to be less than the right side of the last char on a line
  x = max( (off_t)0, x - leftMargin());
  x = min(wordsPerLine()*wordLength - 1 ,x);
  // constrain y to be > topMargin() and less than bottom of last line 
  y = max( (off_t)0, y - topMargin());
  line = min(y/lineSpacing(), (off_t)linesPerPage()-1);
  wordIdx = line*wordsPerLine() + x/wordLength;
  
  off_t byteOffsetInWord = (x%wordLength)*bytesPerWord()/wordLength;
  // =  wordIdx*bytesPerWord + byteOffsetInWord
  return min( (off_t) bytesPerPage()-1, 
	     wordIdx*bytesPerWord() + byteOffsetInWord);

}
//
// event handler implimentation:
//
void HexEditor::setCursorFromXY(int x,int y)
{
  off_t oldWordIdx = localWordOffset();

  _cursor.setOffset( _topLeft+localByteOffsetAtXY(x,y) ,0 );

  // update where the cursor used to be, and where it is now
  if( oldWordIdx != localWordOffset() ) {
    updateWord( oldWordIdx );
  }
  updateWord( localWordOffset() );
  emit offsetChanged(_cursor.byteOffset());
}

void HexEditor::mousePressEvent( QMouseEvent* e )
{
  setCursorFromXY(e->x(),e->y());

  off_t byte_offset = localByteOffset();
  QRect bbox = byteBBox(byte_offset);
  if( e->x() > bbox.right() ) {
    byte_offset++;
  }
  setSelection( SelectionStart, globalOffset( byte_offset ));
}

void HexEditor::mouseMoveEvent( QMouseEvent* e )
{
  setCursorFromXY(e->x(),e->y());

  off_t byte_offset = localByteOffset();
  QRect bbox = byteBBox(byte_offset);
  if(e->x() > bbox.right() ) {
    byte_offset++;
  }

  setSelection( SelectionEnd, globalOffset( byte_offset ));
}

void HexEditor::mouseReleaseEvent( QMouseEvent* e )
{
  setCursorFromXY(e->x(),e->y());

  off_t byte_offset = localByteOffset();
  QRect bbox = byteBBox(byte_offset);
  if(e->x() > bbox.right() ) {
    byte_offset++;
  }

  setSelection( SelectionEnd, globalOffset( byte_offset ));
}

off_t HexEditor::selectionStart() const
{
  if( _selection[SelectionStart] == -1 || _selection[SelectionEnd] == -1 )
    return -1;
  return min(_selection[SelectionStart],_selection[SelectionEnd]);
}
// note: end is open. range is: [start,end)

off_t HexEditor::selectionEnd() const
{
  if( _selection[SelectionStart] == -1 || _selection[SelectionEnd] == -1 )
    return -1;
  return max(_selection[SelectionStart],_selection[SelectionEnd]);
}

void HexEditor::setSelection(SelectionPos_e pos, off_t offset)
{
  if( !_reader.is_open() ) {
    return;
  }
  if( pos == SelectionStart ) {
    _selection[SelectionEnd] = -1;
  } 
  _selection[pos] = offset;

  if( _selection[SelectionStart] < 0 ) {
    emit selectionChanged("");
  } else {
    if( selectionEnd() > -1 && selectionEnd() <= _reader.size() ) {
      QString data;
      for(off_t i = selectionStart();
	  i < selectionEnd();
	  ++i) {
	  data += Translate::ByteToHex(_reader[i]);
      }
      emit selectionChanged( data );
    } else {
      emit selectionChanged( "" );
    }
  }

  repaint();
}

//
// Editor implimentation
//
void HexEditor::keyPressEvent( QKeyEvent *e )
{
  switch ( e->key() ) {
  case Qt::Key_Left:
    cursorLeft();
    break;
  case Qt::Key_Right:
    cursorRight();
    break;
  case Qt::Key_Up:
    cursorUp();
    break;
  case Qt::Key_Down:
    cursorDown();
    break;
  case Qt::Key_PageUp:
    prevPage();
    break;
  case Qt::Key_PageDown:
    nextPage();
    break;
  case Qt::Key_End:
    setTopLeft( _reader.size() - bytesPerPage()/2 );
    break;
  case Qt::Key_Home:
    setTopLeft(0);
    break;
  default:
    e->ignore();
    break;
  }
}

void HexEditor::resizeEvent( QResizeEvent * e )
{
  int height= lineSpacing();
  int totalWordWidth = wordWidth() + wordSpacing();
  int linewidth = e->size().width()/2;

  // don't let _rows or _cols drop below 1
  _rows = max(1,(e->size().height() - _topMargin)/height);
  _cols = max(1,(e->size().width()/2 - _leftMargin)/totalWordWidth);
  
  // now update the line && word bbox vectors
  _lineBBox.reserve(_rows);
  _alineBBox.reserve(_rows);
  _wordBBox.reserve(_rows*_cols);
  _asciiBBox.reserve(_rows*_cols);
  int top,left, aleft;
  for(int r = 0; r < _rows; r++) {
    top = r*height + _topMargin;
    for(int c = 0; c < _cols; c++) {
      left = totalWordWidth*c + _leftMargin;
      aleft = linewidth + totalWordWidth*c + 5;
      _wordBBox[r*_cols+c] = QRect(left,             //left
				   top,              //top
				   totalWordWidth,   //width
				   height);          //height
      _asciiBBox[r*_cols+c] = QRect(aleft, top, totalWordWidth, height);
    }
    _lineBBox[r] = QRect(_leftMargin,top,linewidth,height);
    _alineBBox[r] = QRect(_leftMargin + linewidth , top, e->size().width() - _leftMargin, height);
  }
  // calculate offset label bounding box
  _labelBBox.setRect(0,                        // x
		     0,                        // y
		     _leftMargin,              // width
		     e->size().height());      // height


		     
  // do this to recalculate the amount of displayed data.
  setTopLeft(_topLeft);
  emit rangeChanged(0,_reader.size()/bytesPerLine());
  //emit StepValues(bytesPerLine(), bytesPerPage());
  emit StepValues(1, bytesPerPage()/bytesPerLine());
}
//
// Reimplimented to be more efficient then repainting the whole screen on
// focus events.
//
void HexEditor::focusInEvent( QFocusEvent* ) 
{
  updateWord( localWordOffset() );
}
void HexEditor::focusOutEvent( QFocusEvent* ) 
{
  updateWord( localWordOffset() );
}
// generate's paint events for wordIdx (global wordIdx)
// is safe to call with any wordIdx
void HexEditor::updateWord( off_t wordIdx )
{
  if( wordIdx > -1 && wordIdx < _rows*_cols ) 
    repaint(_wordBBox[wordIdx]);
}

void HexEditor::paintLabels( QPainter* paintPtr) 
{
  // ignore redraw range for first aproximation:
  int y = _wordBBox[0].bottom();
  unsigned int i;
  off_t offset = _topLeft;
  uchar *ucptr;
  //uchar* offsetptr;
  QString label;

  //qDebug() << "original offset:" << offset << "adjusted offset: " << offset + tskptr->offset;
  // offset correction so the offset is showing the valid hex for the object loaded.
  //offset = offset + tskptr->offset;

  for(int row = 0; row < _rows;++row) {
    label = "";
#ifdef WORDS_BIGENDIAN
    for(i=0, ucptr = (uchar*)&offset; i<sizeof(off_t);++i) {
      label += Translate::ByteToHex(*ucptr++);
    }
#else
    // make sure we write offset labels in big-endian (easier to read)
    ucptr = (uchar*)(&offset) + sizeof(off_t)-1;
    for(i=0;i<sizeof(off_t);++i) {
      label += Translate::ByteToHex(*ucptr--);
    }
#endif
    label = label.mid(sizeof(off_t)*2-_offsetLabelBytes);
    paintPtr->drawText( 5, y, label  );
    offset+=bytesPerLine();
    y+=lineSpacing();
  }
  // draw dividing line between offset labels and data
  int x = leftMargin()-fontMaxWidth();
  paintPtr->drawLine(x,topMargin(),
		     x,height()-topMargin());
}
//
// painting optimizations, each time resize is called, it calculates
// the bounding boxes for each word and each line.
// This data can then be retrieved with wordBBox()
// We can then test intersections to see which words need to be redrawn
//
void HexEditor::paintEvent( QPaintEvent* e)
{
/*  QPixmap      pixmap(this->width(),this->height());
    pixmap.fill(backgroundRole());*/
  QPainter     paint( this );//&pixmap);

  // set up painter;/
  paint.setFont(font());
  const QPalette& p = qApp->palette();
  paint.setBrush(p.background());

  if( _labelBBox.intersects(e->rect()) ) {
    paintLabels(&paint);
  }

  QString text;
  if( !getDisplayText(text) ) {
      cerr << "[error] - internal inconsitency. Please file bug report."
	   << endl;
      return;
  }

  // both cursor and selection is drawn underneath the text
  drawCursor( paint );
  drawSelection( paint );
  
  // Find the stop/start row/col idx's for the repaint
  //int totalWordWidth = wordWidth()+1;
  int totalWordWidth = wordWidth()+wordSpacing();
  int row_start = max(0,(e->rect().top()-topMargin())/lineSpacing() );
  int col_start = max(0,(e->rect().left()-leftMargin())/totalWordWidth);
  int row_stop  = min(_rows-1,e->rect().bottom() / lineSpacing());
  int col_stop  = min(_cols-1,(e->rect().right()) / totalWordWidth);

  // draw highlight over the correct text region. need to switch text values to using offset and size
  // for now i'll use the row/col values for testing the highlighting
  //DrawCurrentObject(paint, row_start, row_stop, col_start, col_stop);
  //DrawCurrentObject(paint, e->rect().left(), topMargin(), e->rect().right()/2, height()-topMargin());
  // draw text in repaint event


  // foreach blockaddress in blockaddress
  // if reader.pageidx == blockaddress
  // {
  DrawCharacterFill(paint);
  // }
  drawTextRegion( paint, text, row_start, row_stop, col_start, col_stop );
  // draw ascii text in repaint event
  // draw dividing line
  paint.drawLine(e->rect().right()/2, topMargin(), e->rect().right()/2, height()-topMargin());
  QString ascii;
  getDisplayAscii(ascii);

  totalWordWidth = wordWidth() + wordSpacing();
  row_start = max(0, (e->rect().top() - topMargin())/lineSpacing());
  col_start = max(0, (e->rect().left() - leftMargin() - e->rect().right()/2)/totalWordWidth);
  row_stop = min(_rows-1, e->rect().bottom()/lineSpacing());
  col_stop = min(_cols-1, e->rect().right()/totalWordWidth);

  drawAsciiRegion(paint, ascii, row_start, row_stop, col_start, col_stop);
}

bool HexEditor::getDisplayText( QString& text )
{
  // get data to draw
  switch (_base) {
  case 16:
    Translate::ByteToHex(text,_data);
    break;
  case 8:
    Translate::ByteToOctal(text,_data);
    break;
  case 2:
    Translate::ByteToBinary(text,_data);
    break;
  case -1:
    Translate::ByteToChar(text,_data);
    break;
  default:
    // error state
    return false;
  }
  return true;
}

void HexEditor::getDisplayAscii(QString& txt)
{
    Translate::ByteToChar(txt, _data);
}
//
// accessors for local offsets
//
off_t HexEditor::localByteOffset()  const
{
  return _cursor.byteOffset() - _topLeft;
}
off_t HexEditor::localWordOffset() const
{
  return localByteOffset()/bytesPerWord();
}
// in offset relative to _data[0]
off_t HexEditor::localCharOffset() const
{
  return localByteOffset()*charsPerByte() + _cursor.charOffset();
}
off_t HexEditor::localLineOffset() const
{
  return localWordOffset()/wordsPerLine();
}
int HexEditor::wordWidth() const 
{
  return _fontMaxWidth*charsPerWord();
}
int HexEditor::wordSpacing() const
{
  return _wordSpacing;
}
//
// cursor movement members
//
void HexEditor::seeCursor()
{
  // see if it is already visible
  if ( _cursor.byteOffset() >= _topLeft && 
       _cursor.byteOffset() <= _topLeft+bytesPerPage()-1 ) {
    updateWord( localWordOffset() );
    return;
  } else {
    // setTopLeft so cursor is in middle line of screen
    setTopLeft( max(_cursor.byteOffset() - bytesPerPage()/2, (off_t)0) );
  }
}

void HexEditor::cursorLeft()
{
  off_t oldWordIdx = localWordOffset();
  // move the cursor
  _cursor.decrByChar(2);
  //_cursor.decrByChar(1);
  // make sure the user can see the cursor
  seeCursor();
  // redraw where the cursor used to be
  if( localWordOffset() != oldWordIdx ) 
    updateWord( oldWordIdx );
  emit offsetChanged( _cursor.byteOffset() );
}
void HexEditor::cursorRight()
{
  off_t oldWordIdx = localWordOffset();
  _cursor.incrByChar(2);
  //_cursor.incrByChar(1);
  seeCursor();
  if( localWordOffset() != oldWordIdx ) 
    updateWord( oldWordIdx );
  emit offsetChanged( _cursor.byteOffset() );
}
void HexEditor::cursorUp()
{
  off_t oldWordIdx = localWordOffset();
  _cursor.decrByByte( bytesPerLine() );
  seeCursor();
  if( localWordOffset() != oldWordIdx ) 
    updateWord( oldWordIdx );
  emit offsetChanged( _cursor.byteOffset() );
}
void HexEditor::cursorDown()
{
  off_t oldWordIdx = localWordOffset();
  _cursor.incrByByte( bytesPerLine() );
  seeCursor();
  if( localWordOffset() != oldWordIdx ) 
    updateWord( oldWordIdx );
  emit offsetChanged( _cursor.byteOffset() );
}

void HexEditor::search( const QString& hexText, bool forwards )
{
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  //if( !hexText.length() || _reader.filename() == "" )
  if(!hexText.length() || QString(_reader.filename()).compare("") == 0)
    return;

  vector<uchar> data;
  Translate::HexToByte(data,hexText);
  off_t pos;
  if( forwards ) {
      pos = _reader.findIndex(_cursor.byteOffset(),data,_reader.size()-1);
  } else {
      pos = _reader.rFindIndex(_cursor.byteOffset(),data,0);
  }
  if( pos < _reader.size() ) {
      showMatch(pos,data.size());
      setOffset( forwards?selectionEnd():selectionStart()-1 );
      seeCursor();
  } else {
    QMessageBox::information(this,"Search Failed",
			     "Could not find search data 0x" + hexText);
  }
  QApplication::restoreOverrideCursor();
}
void HexEditor::setBaseASCII() {
  setBase(-1);
}
void HexEditor::setBaseHex()
{
  setBase(16);
}
void HexEditor::setBaseOctal()
{
  setBase(8);
}
void HexEditor::setBaseBinary()
{
  setBase(2);
}
void HexEditor::setBase(int base)
{
  switch(base) {
  case -1:
    // setup ascii editing mode
    //_charsPerByte = 2;
    _charsPerByte = 1;
    break;
  case 2:
    // setup binary editing mode
    _charsPerByte = 8;
    break;
  case 8:
    // setup octal editing mode
    _charsPerByte = 3;
    break;
  case 16:
    // setup hex editing mode
    _charsPerByte = 2;
    break;
  default:
    // just ignore unsupported bases for now
    return;
  }
  _base = base;
  _cursor.setCharsPerByte(_charsPerByte);
  // refresh the display 
  // note: cannot call resize() because it will ignore resize events
  //       if the size has not changed.
  QResizeEvent *re = new QResizeEvent(QSize(size()),QSize(size()));
  resizeEvent(re);
  delete re;
  // make sure we can still see the cursor
  // switching from a larger base to a smaller base has the
  // possibility of pushing the cursor off the screen
  seeCursor();
}

off_t HexEditor::offset() const
{
  return _cursor.byteOffset();
}

Reader * HexEditor::reader()
{
  return &_reader;
}

void HexEditor::showMatch( off_t pos, int len )
{
    setSelection( SelectionStart, pos );
    setSelection( SelectionEnd, pos + len );
}

void HexEditor::drawAsciiRegion(QPainter& paint, const QString& text, int row_start, int row_stop, int col_start, int col_stop)
{
    paint.setPen(qApp->palette().foreground().color());
    paint.setBrush(QColor(0, 255, 0, 15));
    for(int r = row_start; r <= row_stop; r++)
    {
        for(int c = col_start; c <= col_stop; c++)
        {
            int widx = r*_cols+c;
	    paint.drawText(_asciiBBox[widx].left() + wordSpacing(), _asciiBBox[widx].bottom(), text.mid(widx*charsPerWord()/2,charsPerWord()/2));
            //paint.drawRect(_asciiBBox[widx].left(), _asciiBBox[widx].top(), _asciiBBox[widx].left() + wordSpacing(), _asciiBBox[widx].bottom());
        }
    }
}

void HexEditor::drawTextRegion(QPainter& paint, const QString& text, int row_start, int row_stop, int col_start, int col_stop)
{
  paint.setPen(qApp->palette().foreground().color());
  for(int r = row_start; r <= row_stop; r++) {
    for(int c = col_start; c <= col_stop; c++) {
        int widx = r*_cols+c;
        //paint.fillRect(_wordBBox[widx].left() + wordSpacing()/2, _wordBBox[widx].bottom(), widx*charsPerWord(), _wordBBox[widx].top(), QColor(0, 255, 0, 15));
        paint.drawText( _wordBBox[widx].left() + wordSpacing()/2, _wordBBox[widx].bottom(), text.mid(widx*charsPerWord(),charsPerWord()) );
    }
  }
}

void HexEditor::drawSelection( QPainter& paint )
{
  // draw selection
  off_t start = max( (off_t)0, selectionStart() - _topLeft);
  if( start < bytesPerPage() ) {
    off_t stop = min(selectionEnd() - _topLeft, (off_t)bytesPerPage());
    paint.setPen(Qt::NoPen);
    paint.setBrush( qApp->palette().highlight() );
    //paint.setBrush(QColor(0, 0, 255, 15));
    // foreach line with selection
    stop--;
    while( start <= stop ) {
      // linestop = min(stop,endofline)
      off_t linestop = 
	min(stop, start+bytesPerLine()-1 -(start % bytesPerLine()));
      QRect bbox = byteBBox(start);
      QRect abox = abyteBox(start);
      bbox.setRight( byteBBox(linestop).right() );
      abox.setRight(abyteBox(linestop).right());
      paint.drawRect( bbox );
      paint.drawRect(abox);
      start = linestop+1;
    }
  }
  //DrawCurrentObject(paint);
}

// THIS DRAWS ON TOP OF THE TEXT. I MIGHT NEED TO ADD A BRUSH TO WHERE I DRAW TEXT AND THEN DRAW THE TEXT WITH THE HIGHLIGHTING
// IF ITS PART OF THE FILE BYTE OFFSET DATA...
void HexEditor::DrawCurrentObject(QPainter& paint, int row_start, int row_stop, int col_start, int col_stop)
{
    // THAT SORT OF WORKS... A GOOD START TO GO FROM.
    paint.setPen(Qt::NoPen);
    paint.setBrush(QColor(255, 0, 0, 15));
    for(int r = row_start; r <= row_stop; r++)
    {
        for(int c = col_start; c <= col_stop; c++)
        {
            int widx = r*_cols+c;
            //paint.drawRect(_wordBBox[widx].left() + wordSpacing(), _wordBBox[widx].top(), _wordBBox[widx].right()/2, _wordBBox[widx].bottom());
        }
    }
    paint.drawRect(row_start, row_stop, col_start, col_stop);
    /*
     *  // draw selection
  off_t start = max( (off_t)0, selectionStart() - _topLeft);
  if( start < bytesPerPage() ) {
    off_t stop = min(selectionEnd() - _topLeft, (off_t)bytesPerPage());
    paint.setPen(Qt::NoPen);
    paint.setBrush( qApp->palette().highlight() );
    //paint.setBrush(QColor(0, 0, 255, 15));
    // foreach line with selection
    stop--;
    while( start <= stop ) {
      // linestop = min(stop,endofline)
      off_t linestop = 
	min(stop, start+bytesPerLine()-1 -(start % bytesPerLine()));
      QRect bbox = byteBBox(start);
      QRect abox = abyteBox(start);
      bbox.setRight( byteBBox(linestop).right() );
      abox.setRight(abyteBox(linestop).right());
      paint.drawRect( bbox );
      paint.drawRect(abox);
      start = linestop+1;
    }

     */ 
}

void HexEditor::DrawCharacterFill(QPainter& paint)
{
    //paint.setPen(Qt::NoPen);
    //paint.setBrush(QColor(255, 0, 0, 15));
    paint.fillRect(10, 10, 100, 100, QColor(255, 0, 0, 15));
}

void HexEditor::drawCursor( QPainter& paint )
{
  QBrush b = qApp->palette().mid();
  if( localWordOffset() > -1 && localWordOffset() < wordsPerPage() ) {
    if( hasFocus() ) {
      // draw a solid cursor
      paint.setPen(Qt::NoPen);
      paint.setBrush(b);
    } else {
      // just draw the outline
      paint.setPen(b.color());
      paint.setBrush(Qt::NoBrush);
    }
    QRect box = charBBox( localCharOffset() );
    paint.drawRect( box );
  }
}
