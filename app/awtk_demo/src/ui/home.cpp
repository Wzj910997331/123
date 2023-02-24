
#include "home.h"
#include "cviapp_interface.h"
#include "cviapp_ai_handler.h"

widget_t* homeCanvas = NULL;
rect_t faceRect;
static uint32_t __aiUpdateRectTimerID = TK_INVALID_ID;

//#ifdef ENABLE_CVIAI
static CVIAPP_AiDrawRectMsg_S *__pDrawRectMsg[CVIAPP_AIBOX_DEVICE_NUM_MAX];

//#endif

CVI_ERROR_CODE_E getDrawRectByWinId(uint8_t id, rect_t aiRect,
                                    rect_t *drawRect)
{
    int offsetX = 0, offsetY = 0;

    if(NULL == drawRect)
    {
        return RET_FAIL;
    }

    if(id >= CVIAPP_IPC_VIEW_SCREEN_DEFAULT_NUM)
    {
        return RET_FAIL;
    }

    drawRect->w = aiRect.w >> 1;
    drawRect->h = aiRect.h >> 1;

    switch(id)
    {
    case 0:

    break;

    case 1:
        offsetX = (CVIAPP_IPC_VIEW_SCREEN_W >> 1);
    break;

    case 2:
        offsetY = (CVIAPP_IPC_VIEW_SCREEN_H >> 1);
    break;

    case 3:
        offsetX = (CVIAPP_IPC_VIEW_SCREEN_W >> 1);
        offsetY = (CVIAPP_IPC_VIEW_SCREEN_H >> 1);
    break;

    default:
        break;
    }
    drawRect->x = (aiRect.x >> 1) + offsetX;
    drawRect->y = (aiRect.y >> 1) + offsetY;


    return RET_OK;
}


/**
 * Draw the rect in preview
 * mantis.7793 wentao.hu 20220509
 * \input c: the canvas point;
 * \input r: the rect from YUV we will draw;
 * \ret: the enum of ret_t.
 */
static ret_t onCanvasDrawRect(canvas_t *c, int drawX, int drawY, int drawW,
                              int drawH)
{
   // wh_t w = CVIAPP_AI_VIEW_SCREEN_W;
   // wh_t h = CVIAPP_AI_VIEW_SCREEN_H;
    rect_t drawRect;

    if(NULL == c)
    {
        return RET_FAIL;
    }

    if(NULL == c->lcd)
    {
        return RET_FAIL;
    }

    if((0 >= drawW) || (0 >= drawH))
    {
        CVI_NVRLOGD("onCanvasDrawRect get failed r(%d, %d, %d, %d)",
                    drawX, drawY, drawW, drawH);
        drawW = 500;
        drawY = 500;
       // return RET_FAIL;
    }

    /* For 1039, the get scale before we draw it. wentao.hu 20230209 */
    drawRect.x = drawX;//*((float)w/CVIAPP_IPC_VIEW_SCREEN_W);
    drawRect.y = drawY;//*((float)h/CVIAPP_IPC_VIEW_SCREEN_H);
    drawRect.w = drawW;//*((float)w/CVIAPP_IPC_VIEW_SCREEN_W);
    drawRect.h = drawH;//*((float)h/CVIAPP_IPC_VIEW_SCREEN_H);
    //CVI_NVRLOGD("onCanvasDrawRect will draw  r(%d, %d, %d, %d)",
    //                drawRect.x, drawRect.y, drawRect.w, drawRect.h);
    canvas_set_stroke_color(c, color_init(0, 0xff, 0, 0xff));
    canvas_stroke_rect(c, drawRect.x, drawRect.y, drawRect.w, drawRect.h);

    return RET_OK;
}


static ret_t onCanvasPaint(void* ctx, event_t* e)
{
        paint_event_t* evt = (paint_event_t*)e;
        wchar_t *picWChar = L"粤B123456";
        canvas_t* c = evt->c;

        int i = 0, j = 0;
        CVIAPP_AiDrawRectMsg_S *pmsg = NULL;
        rect_t aiRect, drawRect;

        for(i = 0; i < CVIAPP_AIBOX_DEVICE_NUM_MAX; i++)
        {
            pmsg = __pDrawRectMsg[i];
            if(NULL == pmsg)
            {
                continue;
            }

            for(j = 0; j < pmsg->size; j++)
            {
                aiRect.x = pmsg->pr[j].x;
                aiRect.y = pmsg->pr[j].y;
                aiRect.w = pmsg->pr[j].w;
                aiRect.h = pmsg->pr[j].h;

                getDrawRectByWinId(i, aiRect, &drawRect);
                onCanvasDrawRect(c, drawRect.x, drawRect.y, drawRect.w,
                                 drawRect.h);
            }

            if(0 != pmsg->size)
            {
                CVIAPP_AiFreeDrawRectMsg(__pDrawRectMsg[i]);
                __pDrawRectMsg[i]->pIpcView = NULL;
                __pDrawRectMsg[i] = NULL;
            }
        }
        //CVI_NVRLOGD("onCanvasDrawRect get face r(%d, %d, %d, %d)",
        //            faceRect.x, faceRect.y, faceRect.w, faceRect.h);

        /**
         * Draw a box for a car, and show the ID in the box
         * mantis.7793 wentao.hu 20220509
         */
        drawRect.x = 200;
        drawRect.y = 200;
        drawRect.w = 200;
        drawRect.h =200;

        canvas_set_stroke_color(c, color_init(0, 0xff, 0, 0xff));
        canvas_set_stroke_color(c, color_init(0xff, 0, 0, 0xff));

        canvas_set_text_color(c, color_init(0xff, 0x00, 0xff, 0xff));
        canvas_set_font(c, NULL, 32);
        canvas_draw_text_in_rect(c, picWChar, sizeof(picWChar), &drawRect);

        canvas_stroke_rect(c, drawRect.x, drawRect.y, drawRect.w, drawRect.h);

    return RET_OK;
}





ret_t aiUpdateRect(const timer_info_t* timer)
{
    widget_t* win = (widget_t*) timer->ctx;

    widget_invalidate(WIDGET(win), NULL);

    return RET_REPEAT;
}

ret_t aiDrawRectNotify(const idle_info_t* idle)
{
    CVIAPP_AiDrawRectMsg_S *pmsg = (CVIAPP_AiDrawRectMsg_S*) idle->ctx;

    int ch = 0, i = 0, size = 0;

    if (pmsg->pIpcView != NULL)
    {
        widget_t* win = (widget_t*) pmsg->pIpcView;

        __pDrawRectMsg[pmsg->u8Chn] = pmsg;
        ch = pmsg->u8Chn;

        if (NULL != win)
        {
            widget_invalidate(WIDGET(win), NULL);
        }
        else
        {
            CVI_NVRLOGD("check widget getWidget ret NULL");
        }
    }

    return RET_OK;
}


static ret_t onCreate(void* ctx, event_t* e)
{
    printf("onCreate.....\n");

    widget_t* win = WIDGET(ctx);
    widget_t* canvas = canvas_widget_create(win, 0, 0,
                                            CVIAPP_IPC_VIEW_SCREEN_W,
                                            CVIAPP_IPC_VIEW_SCREEN_H);

    widget_set_sensitive(canvas, false);
    widget_on(canvas, EVT_PAINT, onCanvasPaint, win);
    widget_invalidate(win, NULL);

    /* Start ipc Preview*/
    CVIAPP_StartPreview();

    /* To register AI notify function  */
    CVIAPP_AiRegDrawRectNotify(win, 0, aiDrawRectNotify);
    CVIAPP_AiRegDrawRectNotify(win, 1, aiDrawRectNotify);
    CVIAPP_AiRegDrawRectNotify(win, 2, aiDrawRectNotify);
    CVIAPP_AiRegDrawRectNotify(win, 3, aiDrawRectNotify);

    printf("END.....\n");

    return RET_OK;
}

static ret_t onResume(void* ctx, event_t* e)
{
    printf("onResume.....\n");

    return RET_OK;
}

static ret_t onPause(void* ctx, event_t* e)
{
    printf("onPause.....\n");
    return RET_OK;
}

static ret_t onDestroy(void* ctx, event_t* e)
{
    printf("onDestroy.....\n");
    if(TK_INVALID_ID != __aiUpdateRectTimerID)
    {
        timer_remove(__aiUpdateRectTimerID);
        __aiUpdateRectTimerID = TK_INVALID_ID;
    }


    return RET_OK;
}

void open_home_page(void)
{
    widget_t* win = window_open("base");

    widget_on(win, EVT_WINDOW_OPEN, onCreate, win);
    widget_on(win, EVT_WINDOW_TO_FOREGROUND, onResume, win);
    widget_on(win, EVT_WINDOW_TO_BACKGROUND, onPause, win);
    widget_on(win, EVT_WINDOW_CLOSE, onDestroy, win);

}

